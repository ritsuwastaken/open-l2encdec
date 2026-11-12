#include "rsa.h"
#include <algorithm>
#include <atomic>
#include <mutex>
#include <system_error>
#include <thread>
#include <tommath.h>

namespace
{
constexpr size_t NUM_THREADS = 4;
constexpr size_t BLOCK_SIZE = 128;
constexpr size_t BLOCK_BODY_SIZE = 124;

struct Mpi
{
    mp_int v{};
    int status;
    Mpi() : status(mp_init(&v)) {}
    ~Mpi() { mp_clear(&v); }
    Mpi(const Mpi &) = delete;
    Mpi &operator=(const Mpi &) = delete;
};

constexpr size_t align_to_4_bytes(size_t x)
{
    return (x + 3) & ~size_t(3);
}

int mpi_read_hex(Mpi &x, const std::string &hex)
{
    if (x.status != MP_OKAY) return x.status;
    size_t start = !hex.empty() && hex.front() == '-' ? 1 : 0;
    if (start == hex.size() || hex.find_first_not_of("0123456789abcdefABCDEF", start) != std::string::npos)
        return MP_VAL;
    return mp_read_radix(&x.v, hex.c_str(), 16);
}

int mpi_write_block(const mp_int &x, std::vector<unsigned char> &buffer)
{
    size_t size = mp_ubin_size(&x);
    if (size > buffer.size()) return MP_BUF;
    std::fill(buffer.begin(), buffer.end(), 0);
    return mp_to_ubin(&x, buffer.data() + buffer.size() - size, size, nullptr);
}

void store_first_error(std::atomic<int> &err, int rc)
{
    int expected = 0;
    err.compare_exchange_strong(expected, rc);
}

int transform_blocks(std::span<const unsigned char> input,
                     std::vector<unsigned char> &output,
                     const std::string &modulus_hex,
                     const std::string &exponent_hex,
                     bool reject_unreduced_blocks,
                     const rsa::Progress &progress)
{
    Mpi modulus, exponent;
    if (mpi_read_hex(modulus, modulus_hex) != MP_OKAY ||
        mpi_read_hex(exponent, exponent_hex) != MP_OKAY ||
        mp_cmp_d(&modulus.v, 0) != MP_GT || mp_iseven(&modulus.v) || mp_isneg(&exponent.v))
        return -2;

    output.resize(input.size());
    if (input.empty()) return 0;

    const size_t total_blocks = input.size() / BLOCK_SIZE;
    if (progress && !progress(0, total_blocks))
        return rsa::CANCELLED;

    std::atomic<size_t> next_block{0};
    std::atomic<size_t> completed{0};
    std::atomic<int> error{0};
    std::mutex progress_mutex;

    auto report = [&]()
    {
        if (!progress || !progress_mutex.try_lock()) return;
        std::lock_guard lock(progress_mutex, std::adopt_lock);
        if (error.load() == 0 && !progress(completed.load(), total_blocks))
            store_first_error(error, rsa::CANCELLED);
    };
    auto worker = [&]()
    {
        Mpi block, result;
        if (block.status != MP_OKAY)
        {
            store_first_error(error, block.status);
            return;
        }
        if (result.status != MP_OKAY)
        {
            store_first_error(error, result.status);
            return;
        }
        std::vector<unsigned char> buffer(BLOCK_SIZE);

        while (error.load() == 0)
        {
            const size_t i = next_block.fetch_add(1);
            if (i >= total_blocks) break;
            const size_t offset = i * BLOCK_SIZE;
            int rc = mp_from_ubin(&block.v, input.data() + offset, BLOCK_SIZE);
            if (rc == MP_OKAY && reject_unreduced_blocks && mp_cmp(&block.v, &modulus.v) != MP_LT)
                rc = MP_VAL;
            if (rc == MP_OKAY)
                rc = mp_exptmod(&block.v, &exponent.v, &modulus.v, &result.v);
            if (rc == MP_OKAY)
                rc = mpi_write_block(result.v, buffer);
            if (rc != MP_OKAY)
            {
                store_first_error(error, rc);
                return;
            }
            std::copy(buffer.begin(), buffer.end(), output.begin() + offset);
            completed.fetch_add(1);
            report();
        }
    };

    const size_t hw = std::thread::hardware_concurrency();
    const size_t num_threads = std::min({hw ? hw : NUM_THREADS, NUM_THREADS, total_blocks});
    {
        std::vector<std::jthread> threads;
        threads.reserve(num_threads - 1);
        try
        {
            for (size_t i = 1; i < num_threads; ++i)
                threads.emplace_back(worker);
        }
        catch (const std::system_error &)
        {
            // ignore
        }
        worker();
    }
    if (progress && error.load() == 0)
        progress(total_blocks, total_blocks);
    return error.load();
}
} // namespace

size_t rsa::add_padding(std::vector<uint8_t> &output, std::span<const uint8_t> input)
{
    size_t input_offset = 0;
    size_t num_blocks = (input.size() + BLOCK_BODY_SIZE - 1) / BLOCK_BODY_SIZE;
    output.resize(num_blocks * BLOCK_SIZE);

    for (size_t output_offset = 0; input_offset < input.size(); output_offset += BLOCK_SIZE)
    {
        size_t chunk_size = std::min(input.size() - input_offset, BLOCK_BODY_SIZE);
        std::fill(output.begin() + output_offset, output.begin() + output_offset + BLOCK_SIZE, 0);
        output[output_offset + 3] = static_cast<uint8_t>(chunk_size);
        size_t data_offset = output_offset + BLOCK_SIZE - align_to_4_bytes(chunk_size);
        std::copy(input.begin() + input_offset, input.begin() + input_offset + chunk_size,
                  output.begin() + data_offset);
        input_offset += chunk_size;
    }

    return output.size();
}

size_t rsa::remove_padding(std::vector<uint8_t> &output, std::span<const uint8_t> input)
{
    output.clear();

    for (size_t offset = 0; offset + BLOCK_SIZE <= input.size(); offset += BLOCK_SIZE)
    {
        size_t size_byte = input[offset + 3];
        size_t chunk_size = std::min(size_byte, BLOCK_BODY_SIZE);
        size_t data_offset = offset + BLOCK_SIZE - align_to_4_bytes(chunk_size);
        if (data_offset + chunk_size > input.size()) break;
        output.insert(output.end(), input.begin() + data_offset,
                      input.begin() + data_offset + chunk_size);
    }

    return output.size();
}

int rsa::encrypt(std::span<const unsigned char> input_data,
                 std::vector<unsigned char> &output_data,
                 const std::string &modulus_hex,
                 const std::string &public_exp_hex,
                 const Progress &progress)
{
    std::vector<unsigned char> padded;
    if (add_padding(padded, input_data) == 0) return -1;
    const int rc = transform_blocks(padded, output_data, modulus_hex, public_exp_hex, false, progress);
    if (rc != MP_OKAY) output_data.clear();
    return rc;
}

int rsa::decrypt(std::span<const unsigned char> input_data,
                 std::vector<unsigned char> &output_data,
                 const std::string &modulus_hex,
                 const std::string &private_exp_hex,
                 const Progress &progress)
{
    if (input_data.size() % BLOCK_SIZE != 0) return -1;
    std::vector<unsigned char> decrypted;
    const int rc = transform_blocks(input_data, decrypted, modulus_hex, private_exp_hex, true, progress);
    if (rc != MP_OKAY) return rc;
    remove_padding(output_data, decrypted);
    return 0;
}
