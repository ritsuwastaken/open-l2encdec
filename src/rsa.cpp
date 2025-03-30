#include "mbedtls/bignum.h"
#include "rsa.h"
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <algorithm>

#define ALIGN_TO_4_BYTES(x) (((x) + 3) & ~3)

const size_t NUM_THREADS = 4;
const size_t BLOCK_SIZE = 128;
const size_t BLOCK_BODY_SIZE = 124;

size_t add_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
{
    size_t input_offset = 0, output_offset = 0;
    output.resize((input.size() + BLOCK_BODY_SIZE - 1) / BLOCK_BODY_SIZE * BLOCK_SIZE);

    while (input_offset < input.size())
    {
        size_t data_size = std::min<size_t>(input.size() - input_offset, BLOCK_BODY_SIZE);
        std::fill(output.begin() + output_offset, output.begin() + output_offset + BLOCK_SIZE, 0);

        output[output_offset + 3] = static_cast<uint8_t>(data_size);
        size_t padding_end = output_offset + BLOCK_SIZE - ALIGN_TO_4_BYTES(data_size);
        std::copy(input.begin() + input_offset, input.begin() + input_offset + data_size, output.begin() + padding_end);

        input_offset += data_size;
        output_offset += BLOCK_SIZE;
    }

    return output_offset;
}

size_t remove_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
{
    size_t output_size = 0;
    output.resize(input.size());

    for (int offset = 0; offset < input.size(); offset += BLOCK_SIZE)
    {
        size_t block_size_offset = offset + 3;
        if (block_size_offset >= input.size())
            break;

        size_t data_size = std::min<size_t>(input[block_size_offset], BLOCK_BODY_SIZE);
        size_t padding_end = offset + BLOCK_SIZE - ALIGN_TO_4_BYTES(data_size);
        if (padding_end + data_size > input.size())
            break;

        std::copy(input.begin() + padding_end, input.begin() + padding_end + data_size, output.begin() + output_size);
        output_size += data_size;
    }

    output.resize(output_size);
    return output_size;
}

static void mpi_read_hex(mbedtls_mpi *x, const std::string &hex)
{
    mbedtls_mpi_read_string(x, 16, hex.c_str());
}

void RSA::encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex)
{
    std::vector<unsigned char> padded_buffer;
    size_t input_size = add_padding(padded_buffer, input_data);

    mbedtls_mpi modulus, public_exp;
    mbedtls_mpi_init(&modulus);
    mbedtls_mpi_init(&public_exp);
    mpi_read_hex(&modulus, modulus_hex);
    mpi_read_hex(&public_exp, public_exp_hex);

    output_data.resize(input_size);
    std::vector<unsigned char> encrypted_buffer(input_size);
    size_t total_blocks = input_size / BLOCK_SIZE;

    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0)
        num_threads = NUM_THREADS;
    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);

    auto encrypt_block = [&]()
    {
        mbedtls_mpi block, encrypted_block;
        mbedtls_mpi_init(&block);
        mbedtls_mpi_init(&encrypted_block);

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;

            mbedtls_mpi_read_binary(&block, padded_buffer.data() + offset, BLOCK_SIZE);
            mbedtls_mpi_exp_mod(&encrypted_block, &block, &public_exp, &modulus, nullptr);
            std::vector<unsigned char> temp(BLOCK_SIZE, 0);
            mbedtls_mpi_write_binary(&encrypted_block, temp.data(), BLOCK_SIZE);
            std::copy(temp.begin(), temp.end(), encrypted_buffer.begin() + offset);
        }

        mbedtls_mpi_free(&block);
        mbedtls_mpi_free(&encrypted_block);
    };

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(encrypt_block);
    }
    for (auto &t : threads)
        t.join();

    mbedtls_mpi_free(&modulus);
    mbedtls_mpi_free(&public_exp);

    output_data.assign(encrypted_buffer.begin(), encrypted_buffer.end());
}

int RSA::decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex)
{
    if (input_data.size() % BLOCK_SIZE != 0)
        return -1;

    mbedtls_mpi modulus, private_exp;
    mbedtls_mpi_init(&modulus);
    mbedtls_mpi_init(&private_exp);
    mpi_read_hex(&modulus, modulus_hex);
    mpi_read_hex(&private_exp, private_exp_hex);

    std::vector<unsigned char> decrypted_buffer(input_data.size());
    size_t total_blocks = input_data.size() / BLOCK_SIZE;

    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0)
        num_threads = NUM_THREADS;
    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);

    auto decrypt_block = [&]()
    {
        mbedtls_mpi block, decrypted_block;
        mbedtls_mpi_init(&block);
        mbedtls_mpi_init(&decrypted_block);

        std::vector<unsigned char> temp(BLOCK_SIZE, 0);
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
    {
        threads.emplace_back(decrypt_block);
    }
    for (auto &t : threads)
        t.join();

    mbedtls_mpi_free(&modulus);
    mbedtls_mpi_free(&private_exp);

    output_data.resize(decrypted_buffer.size());
    remove_padding(output_data, decrypted_buffer);

    return 0;
}
