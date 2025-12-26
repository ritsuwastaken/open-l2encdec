#include "rsa.h"
#include <algorithm>
#include <atomic>
#include <tommath.h>

#if L2ENCDEC_ENABLE_THREADS
#include <thread>
#endif

namespace
{
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

int transform_blocks(const std::vector<unsigned char> &input,
                     std::vector<unsigned char> &output,
                     const std::string &modulus_hex,
                     const std::string &exponent_hex)
{
    Mpi modulus, exponent;
    if (mpi_read_hex(modulus, modulus_hex) != MP_OKAY ||
        mpi_read_hex(exponent, exponent_hex) != MP_OKAY ||
        mp_cmp_d(&modulus.v, 0) != MP_GT || mp_iseven(&modulus.v) || mp_isneg(&exponent.v))
        return -2;

    output.resize(input.size());
    const size_t total_blocks = input.size() / BLOCK_SIZE;
    std::atomic<size_t> next_block{0};
    std::atomic<int> error{0};
    auto worker = [&]()
    {
        auto fail = [&](int rc)
        {
            int expected = 0;
            error.compare_exchange_strong(expected, rc);
        };
        Mpi block, result;
        if (block.status != MP_OKAY) { fail(block.status); return; }
        if (result.status != MP_OKAY) { fail(result.status); return; }
        std::vector<unsigned char> buffer(BLOCK_SIZE);

        while (error.load() == 0)
        {
            const size_t i = next_block.fetch_add(1);
            if (i >= total_blocks) break;
            const size_t offset = i * BLOCK_SIZE;
            int rc = mp_from_ubin(&block.v, input.data() + offset, BLOCK_SIZE);
            if (rc == MP_OKAY)
                rc = mp_exptmod(&block.v, &exponent.v, &modulus.v, &result.v);
            if (rc == MP_OKAY)
                rc = mpi_write_block(result.v, buffer);
            if (rc != MP_OKAY) { fail(rc); return; }
            std::copy(buffer.begin(), buffer.end(), output.begin() + offset);
        }
    };

#if L2ENCDEC_ENABLE_THREADS
    {
        const size_t hw = std::thread::hardware_concurrency();
        const size_t count = std::min({hw ? hw : size_t(4), size_t(4), total_blocks});
        std::vector<std::jthread> threads;
        for (size_t i = 1; i < count; ++i)
            threads.emplace_back(worker);
        worker();
    }
#else
    worker();
#endif
    return error.load();
}
} // namespace

size_t rsa::add_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
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

size_t rsa::remove_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
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

int rsa::encrypt(const std::vector<unsigned char> &input_data,
                 std::vector<unsigned char> &output_data,
                 const std::string &modulus_hex,
                 const std::string &public_exp_hex)
{
    std::vector<unsigned char> padded;
    if (add_padding(padded, input_data) == 0) return -1;
    const int rc = transform_blocks(padded, output_data, modulus_hex, public_exp_hex);
    if (rc != MP_OKAY) output_data.clear();
    return rc;
}

int rsa::decrypt(const std::vector<unsigned char> &input_data,
                 std::vector<unsigned char> &output_data,
                 const std::string &modulus_hex,
                 const std::string &private_exp_hex)
{
    if (input_data.size() % BLOCK_SIZE != 0) return -1;
    std::vector<unsigned char> decrypted;
    const int rc = transform_blocks(input_data, decrypted, modulus_hex, private_exp_hex);
    if (rc != MP_OKAY) return rc;
    remove_padding(output_data, decrypted);
    return 0;
}
