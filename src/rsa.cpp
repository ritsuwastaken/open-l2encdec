#include "rsa.h"
#include <mbedtls/bignum.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <algorithm>

#define ALIGN_TO_4_BYTES(x) (((x) + 3) & ~3)

constexpr size_t NUM_THREADS = 4;
constexpr size_t BLOCK_SIZE = 128;
constexpr size_t BLOCK_BODY_SIZE = 124;

size_t add_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
{
    size_t input_offset = 0;
    size_t num_blocks = (input.size() + BLOCK_BODY_SIZE - 1) / BLOCK_BODY_SIZE;
    output.resize(num_blocks * BLOCK_SIZE);

    for (size_t output_offset = 0; input_offset < input.size(); output_offset += BLOCK_SIZE)
    {
        size_t chunk_size = std::min(input.size() - input_offset, BLOCK_BODY_SIZE);
        std::fill(output.begin() + output_offset, output.begin() + output_offset + BLOCK_SIZE, 0);

        output[output_offset + 3] = static_cast<uint8_t>(chunk_size);
        size_t data_offset = output_offset + BLOCK_SIZE - ALIGN_TO_4_BYTES(chunk_size);
        std::copy(input.begin() + input_offset, input.begin() + input_offset + chunk_size, output.begin() + data_offset);

        input_offset += chunk_size;
    }

    return output.size();
}

size_t remove_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
{
    output.clear();

    for (size_t offset = 0; offset + BLOCK_SIZE <= input.size(); offset += BLOCK_SIZE)
    {
        size_t size_byte = input[offset + 3];
        size_t chunk_size = std::min(size_byte, BLOCK_BODY_SIZE);
        size_t data_offset = offset + BLOCK_SIZE - ALIGN_TO_4_BYTES(chunk_size);

        if (data_offset + chunk_size > input.size())
            break;

        output.insert(output.end(), input.begin() + data_offset, input.begin() + data_offset + chunk_size);
    }

    return output.size();
}

static void mpi_read_hex(mbedtls_mpi *x, const std::string &hex)
{
    mbedtls_mpi_read_string(x, 16, hex.c_str());
}

void RSA::encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex)
{
    std::vector<unsigned char> padded_buffer;
    size_t total_size = add_padding(padded_buffer, input_data);
    size_t total_blocks = total_size / BLOCK_SIZE;

    mbedtls_mpi modulus, public_exp;
    mbedtls_mpi_init(&modulus);
    mbedtls_mpi_init(&public_exp);
    mpi_read_hex(&modulus, modulus_hex);
    mpi_read_hex(&public_exp, public_exp_hex);

    output_data.resize(total_size);

    size_t num_threads = std::min(std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : NUM_THREADS, std::max<size_t>(1, total_blocks / 4));

    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);

    auto encrypt_block = [&]()
    {
        mbedtls_mpi block, encrypted_block;
        mbedtls_mpi_init(&block);
        mbedtls_mpi_init(&encrypted_block);
        std::vector<unsigned char> temp(BLOCK_SIZE);

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;
            mbedtls_mpi_read_binary(&block, padded_buffer.data() + offset, BLOCK_SIZE);
            mbedtls_mpi_exp_mod(&encrypted_block, &block, &public_exp, &modulus, nullptr);
            mbedtls_mpi_write_binary(&encrypted_block, temp.data(), BLOCK_SIZE);
            std::copy(temp.begin(), temp.end(), output_data.begin() + offset);
        }

        mbedtls_mpi_free(&block);
        mbedtls_mpi_free(&encrypted_block);
    };

    threads.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i)
        threads.emplace_back(encrypt_block);
    for (auto &t : threads)
        t.join();

    mbedtls_mpi_free(&modulus);
    mbedtls_mpi_free(&public_exp);
}

int RSA::decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex)
{
    if (input_data.size() % BLOCK_SIZE != 0)
        return -1;

    size_t total_blocks = input_data.size() / BLOCK_SIZE;
    std::vector<unsigned char> decrypted_buffer(input_data.size());

    mbedtls_mpi modulus, private_exp;
    mbedtls_mpi_init(&modulus);
    mbedtls_mpi_init(&private_exp);
    mpi_read_hex(&modulus, modulus_hex);
    mpi_read_hex(&private_exp, private_exp_hex);

    size_t num_threads = std::min(std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : NUM_THREADS, std::max<size_t>(1, total_blocks / 4));

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    std::atomic<size_t> next_block(0);

    auto decrypt_block = [&]()
    {
        mbedtls_mpi block, decrypted_block;
        mbedtls_mpi_init(&block);
        mbedtls_mpi_init(&decrypted_block);
        std::vector<unsigned char> temp(BLOCK_SIZE);

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;
            mbedtls_mpi_read_binary(&block, input_data.data() + offset, BLOCK_SIZE);
            mbedtls_mpi_exp_mod(&decrypted_block, &block, &private_exp, &modulus, nullptr);
            mbedtls_mpi_write_binary(&decrypted_block, temp.data(), BLOCK_SIZE);
            std::copy(temp.begin(), temp.end(), decrypted_buffer.begin() + offset);
        }

        mbedtls_mpi_free(&block);
        mbedtls_mpi_free(&decrypted_block);
    };

    for (size_t i = 0; i < num_threads; ++i)
        threads.emplace_back(decrypt_block);
    for (auto &t : threads)
        t.join();

    mbedtls_mpi_free(&modulus);
    mbedtls_mpi_free(&private_exp);

    output_data.clear();
    remove_padding(output_data, decrypted_buffer);
    return 0;
}
