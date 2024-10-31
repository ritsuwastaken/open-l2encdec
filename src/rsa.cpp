#ifdef GMP_FOUND
#include <gmp.h>
#else
#include <mini-gmp.h>
#endif

#include "rsa.h"
#include <thread>
#include <iomanip>

#define ALIGN_TO_4_BYTES(x) (((x) + 3) & ~3)

const size_t NUM_THREADS = 4;
const size_t BLOCK_SIZE = 128;
const size_t BLOCK_BODY_SIZE = 124;

size_t add_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input)
{
    size_t input_offset = 0;
    size_t output_offset = 0;
    output.resize((input.size() + BLOCK_BODY_SIZE - 1) / BLOCK_BODY_SIZE * BLOCK_SIZE);

    while (input_offset < input.size())
    {
        size_t data_size = std::min<size_t>(input.size() - input_offset, BLOCK_BODY_SIZE);
        std::fill(output.begin() + output_offset,
                  output.begin() + output_offset + BLOCK_SIZE, 0);

        output[output_offset + 3] = static_cast<uint8_t>(data_size);
        size_t padding_end = output_offset + BLOCK_SIZE - ALIGN_TO_4_BYTES(data_size);
        std::copy(input.begin() + input_offset,
                  input.begin() + input_offset + data_size,
                  output.begin() + padding_end);

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

        std::copy(input.begin() + padding_end,
                  input.begin() + padding_end + data_size,
                  output.begin() + output_size);
        output_size += data_size;
    }

    output.resize(output_size);
    return output_size;
}

void RSA::encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex)
{
    std::vector<unsigned char> padded_buffer;
    int input_size = add_padding(padded_buffer, input_data);

    mpz_t modulus, public_exp;
    mpz_init(modulus);
    mpz_init(public_exp);
    mpz_set_str(modulus, modulus_hex.c_str(), 16);
    mpz_set_str(public_exp, public_exp_hex.c_str(), 16);

    output_data.resize(input_size);
    std::vector<unsigned char> encrypted_buffer(input_size);
    size_t total_blocks = (input_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    mpz_t block, encrypted_block;
    mpz_init(block);
    mpz_init(encrypted_block);

    size_t hardware_concurrency = std::thread::hardware_concurrency();
    size_t num_threads = hardware_concurrency > 0 ? hardware_concurrency : NUM_THREADS;
    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);
    std::atomic<size_t> completed_blocks(0);

    auto encrypt_block = [&]()
    {
        mpz_t block, encrypted_block;
        mpz_init(block);
        mpz_init(encrypted_block);

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;
            size_t count;

            mpz_import(block, BLOCK_SIZE / 4, 1, 4, 1, 0, padded_buffer.data() + offset);
            mpz_powm(encrypted_block, block, public_exp, modulus);
            mpz_export(encrypted_buffer.data() + offset, &count, 1, 4, 1, 0, encrypted_block);

            completed_blocks.fetch_add(1);
        }

        mpz_clear(block);
        mpz_clear(encrypted_block);
    };

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(encrypt_block);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    mpz_clear(block);
    mpz_clear(encrypted_block);
    mpz_clear(modulus);
    mpz_clear(public_exp);

    output_data.assign(encrypted_buffer.begin(), encrypted_buffer.end());
}

int RSA::decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex)
{
    size_t file_size = input_data.size();
    if ((file_size % BLOCK_SIZE) != 0)
        return -1;

    mpz_t modulus, private_exp;
    mpz_init(modulus);
    mpz_init(private_exp);
    mpz_set_str(modulus, modulus_hex.c_str(), 16);
    mpz_set_str(private_exp, private_exp_hex.c_str(), 16);

    std::vector<unsigned char> decrypted_buffer(file_size);
    size_t total_blocks = file_size / BLOCK_SIZE;

    size_t hardware_concurrency = std::thread::hardware_concurrency();
    size_t num_threads = hardware_concurrency > 0 ? hardware_concurrency : NUM_THREADS;
    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);
    std::atomic<size_t> completed_blocks(0);

    auto decrypt_block = [&]()
    {
        mpz_t block, decrypted_block;
        mpz_init(block);
        mpz_init(decrypted_block);

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;
            size_t count;

            mpz_import(block, BLOCK_SIZE / 4, 1, 4, 1, 0, input_data.data() + offset);
            mpz_powm(decrypted_block, block, private_exp, modulus);
            mpz_export(decrypted_buffer.data() + offset, &count, 1, 4, 1, 0, decrypted_block);

            completed_blocks.fetch_add(1);
        }

        mpz_clear(block);
        mpz_clear(decrypted_block);
    };

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(decrypt_block);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    mpz_clear(modulus);
    mpz_clear(private_exp);

    output_data.resize(file_size);
    remove_padding(output_data, decrypted_buffer);

    return 0;
}
