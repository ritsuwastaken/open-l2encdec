#include "xor.h"
#include <algorithm>

int XOR::get_key_by_index(int index)
{
    int d1 = index & 0xf;
    int d2 = (index >> 4) & 0xf;
    int d3 = (index >> 8) & 0xf;
    int d4 = (index >> 12) & 0xf;
    return ((d2 ^ d4) << 4) | (d1 ^ d3);
}

int XOR::get_key_by_filename(std::string filename)
{
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    int acc = 0;
    for (char c : filename)
        acc += static_cast<int>(c);
    return acc & 0xff;
}

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
