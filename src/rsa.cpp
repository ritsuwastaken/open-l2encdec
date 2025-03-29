#include <openssl/bn.h>
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

void L2RSA::encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex)
{
    std::vector<unsigned char> padded_buffer;
    size_t input_size = add_padding(padded_buffer, input_data);

    BIGNUM *modulus = BN_new();
    BIGNUM *public_exp = BN_new();
    BN_CTX *ctx = BN_CTX_new();
    
    BN_hex2bn(&modulus, modulus_hex.c_str());
    BN_hex2bn(&public_exp, public_exp_hex.c_str());

    output_data.resize(input_size);
    std::vector<unsigned char> encrypted_buffer(input_size);
    size_t total_blocks = (input_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    size_t hardware_concurrency = std::thread::hardware_concurrency();
    size_t num_threads = hardware_concurrency > 0 ? hardware_concurrency : NUM_THREADS;
    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);
    std::atomic<size_t> completed_blocks(0);

    auto encrypt_block = [&]()
    {
        BIGNUM *block = BN_new();
        BIGNUM *encrypted_block = BN_new();
        BN_CTX *local_ctx = BN_CTX_new();

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;
            
            BN_bin2bn(padded_buffer.data() + offset, BLOCK_SIZE, block);
            BN_mod_exp(encrypted_block, block, public_exp, modulus, local_ctx);
            BN_bn2bin(encrypted_block, encrypted_buffer.data() + offset);
            completed_blocks.fetch_add(1);
        }

        BN_free(block);
        BN_free(encrypted_block);
        BN_CTX_free(local_ctx);
    };

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(encrypt_block);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    BN_free(modulus);
    BN_free(public_exp);
    BN_CTX_free(ctx);

    output_data.assign(encrypted_buffer.begin(), encrypted_buffer.end());
}

int L2RSA::decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex)
{
    size_t file_size = input_data.size();
    if ((file_size % BLOCK_SIZE) != 0)
        return -1;

    BIGNUM *modulus = BN_new();
    BIGNUM *private_exp = BN_new();
    BN_CTX *ctx = BN_CTX_new();
    
    BN_hex2bn(&modulus, modulus_hex.c_str());
    BN_hex2bn(&private_exp, private_exp_hex.c_str());

    std::vector<unsigned char> decrypted_buffer(file_size);
    size_t total_blocks = file_size / BLOCK_SIZE;

    size_t hardware_concurrency = std::thread::hardware_concurrency();
    size_t num_threads = hardware_concurrency > 0 ? hardware_concurrency : NUM_THREADS;
    std::vector<std::thread> threads;
    std::atomic<size_t> next_block(0);
    std::atomic<size_t> completed_blocks(0);

    auto decrypt_block = [&]()
    {
        BIGNUM *block = BN_new();
        BIGNUM *decrypted_block = BN_new();
        BN_CTX *local_ctx = BN_CTX_new();
        std::vector<unsigned char> temp_buffer(BLOCK_SIZE);

        size_t i;
        while ((i = next_block.fetch_add(1)) < total_blocks)
        {
            size_t offset = i * BLOCK_SIZE;
            
            std::copy(input_data.begin() + offset, 
                     input_data.begin() + offset + BLOCK_SIZE,
                     temp_buffer.begin());
            BN_bin2bn(temp_buffer.data(), BLOCK_SIZE, block);
            
            BN_mod_exp(decrypted_block, block, private_exp, modulus, local_ctx);
            
            std::fill(temp_buffer.begin(), temp_buffer.end(), 0);
            int bytes_written = BN_bn2bin(decrypted_block, temp_buffer.data());
            
            if (bytes_written < BLOCK_SIZE)
            {
                std::copy_backward(temp_buffer.begin(), 
                                 temp_buffer.begin() + bytes_written,
                                 temp_buffer.begin() + BLOCK_SIZE);
                std::fill(temp_buffer.begin(), 
                         temp_buffer.begin() + (BLOCK_SIZE - bytes_written), 
                         0);
            }
            
            std::copy(temp_buffer.begin(), temp_buffer.end(),
                     decrypted_buffer.begin() + offset);

            completed_blocks.fetch_add(1);
        }

        BN_free(block);
        BN_free(decrypted_block);
        BN_CTX_free(local_ctx);
    };

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(decrypt_block);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    BN_free(modulus);
    BN_free(private_exp);
    BN_CTX_free(ctx);

    output_data.resize(file_size);
    remove_padding(output_data, decrypted_buffer);

    return 0;
}
