#include "rsa.h"
#include <algorithm>
#include <atomic>
#include <mbedtls/bignum.h>

namespace
{
constexpr size_t NUM_THREADS = 4;
constexpr size_t BLOCK_SIZE = 128;
constexpr size_t BLOCK_BODY_SIZE = 124;

struct Mpi
{
    mbedtls_mpi v;
    Mpi() { mbedtls_mpi_init(&v); }
    ~Mpi() { mbedtls_mpi_free(&v); }
    Mpi(const Mpi &) = delete;
    Mpi &operator=(const Mpi &) = delete;
};

constexpr size_t align_to_4_bytes(size_t x)
{
    return (x + 3) & ~size_t(3);
}

int mpi_read_hex(mbedtls_mpi *x, const std::string &hex)
{
    return mbedtls_mpi_read_string(x, 16, hex.c_str());
}

inline void store_first_error(std::atomic<int> &err, int rc)
{
    int expected = 0;
    err.compare_exchange_strong(expected, rc);
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
    std::vector<unsigned char> padded_buffer;
    size_t total_size = add_padding(padded_buffer, input_data);
    if (total_size == 0) return -1;

    size_t total_blocks = total_size / BLOCK_SIZE;
    output_data.resize(total_size);

    Mpi modulus, public_exp;
    if (mpi_read_hex(&modulus.v, modulus_hex) != 0 ||
        mpi_read_hex(&public_exp.v, public_exp_hex) != 0)
    {
        output_data.clear();
        return -2;
    }

    Mpi block, encrypted_block;
    std::vector<unsigned char> temp(BLOCK_SIZE);

    for (size_t i = 0; i < total_blocks; ++i)
    {
        size_t offset = i * BLOCK_SIZE;

        int rc = mbedtls_mpi_read_binary(&block.v,
                                         padded_buffer.data() + offset,
                                         BLOCK_SIZE);
        if (rc != 0) return rc;

        rc = mbedtls_mpi_exp_mod(&encrypted_block.v,
                                 &block.v,
                                 &public_exp.v,
                                 &modulus.v,
                                 nullptr);
        if (rc != 0) return rc;

        rc = mbedtls_mpi_write_binary(&encrypted_block.v,
                                      temp.data(),
                                      BLOCK_SIZE);
        if (rc != 0) return rc;

        std::copy(temp.begin(), temp.end(),
                  output_data.begin() + offset);
    }

    return 0;
}

int rsa::decrypt(const std::vector<unsigned char> &input_data,
                 std::vector<unsigned char> &output_data,
                 const std::string &modulus_hex,
                 const std::string &private_exp_hex)
{
    if (input_data.size() % BLOCK_SIZE != 0)
        return -1;

    size_t total_blocks = input_data.size() / BLOCK_SIZE;
    std::vector<unsigned char> decrypted_buffer(input_data.size());

    Mpi modulus, private_exp;
    if (mpi_read_hex(&modulus.v, modulus_hex) != 0 ||
        mpi_read_hex(&private_exp.v, private_exp_hex) != 0)
    {
        return -2;
    }

    Mpi block, decrypted_block;
    std::vector<unsigned char> temp(BLOCK_SIZE);

    for (size_t i = 0; i < total_blocks; ++i)
    {
        size_t offset = i * BLOCK_SIZE;

        int rc = mbedtls_mpi_read_binary(&block.v,
                                         input_data.data() + offset,
                                         BLOCK_SIZE);
        if (rc != 0) return rc;

        rc = mbedtls_mpi_exp_mod(&decrypted_block.v,
                                 &block.v,
                                 &private_exp.v,
                                 &modulus.v,
                                 nullptr);
        if (rc != 0) return rc;

        rc = mbedtls_mpi_write_binary(&decrypted_block.v,
                                      temp.data(),
                                      BLOCK_SIZE);
        if (rc != 0) return rc;

        std::copy(temp.begin(), temp.end(),
                  decrypted_buffer.begin() + offset);
    }

    output_data.clear();
    remove_padding(output_data, decrypted_buffer);
    return 0;
}
