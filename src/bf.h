#ifndef BF_H
#define BF_H

#include <iostream>
#include <vector>
#include <cstdint>

namespace BF
{
    size_t encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const unsigned char *key, int key_size);
    size_t decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const unsigned char *key, int key_size);
}

#endif // BF_H
