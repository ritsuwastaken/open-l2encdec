#ifndef XOR_H
#define XOR_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <functional>

namespace XOR
{
    using KeyGenerator = std::function<int(int)>;

    size_t encrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int xor_key);
    size_t encrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int start_index, KeyGenerator key_generator);
    size_t decrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int xor_key);
    size_t decrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int start_index, KeyGenerator key_generator);
}

#endif // XOR_H
