#include "xor.h"

size_t XOR::encrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int xor_key)
{
    output.assign(input.begin(), input.end());
    for (auto &byte : output)
    {
        byte ^= xor_key;
    }
    return output.size();
}

size_t XOR::decrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int xor_key)
{
    output.assign(input.begin(), input.end());
    for (auto &byte : output)
    {
        if (byte >= 0)
            byte ^= xor_key;
    }
    return output.size();
}

size_t XOR::encrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int start_index, KeyGenerator key_generator)
{
    int ind = start_index;
    output.assign(input.begin(), input.end());
    for (auto &byte : output)
    {
        byte ^= key_generator(ind++);
    }
    return output.size();
}

size_t XOR::decrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int start_index, KeyGenerator key_generator)
{
    int ind = start_index;
    output.assign(input.begin(), input.end());
    for (auto &byte : output)
    {
        byte ^= key_generator(ind++);
    }
    return output.size();
}
