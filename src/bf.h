#ifndef BF_H
#define BF_H

#include <cstddef>
#include <vector>

namespace BF
{
size_t encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const unsigned char *key, int key_size);
size_t decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const unsigned char *key, int key_size);
} // namespace BF

#endif // BF_H
