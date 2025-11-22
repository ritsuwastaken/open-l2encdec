#ifndef BF_H
#define BF_H

#include <cstddef>
#include <vector>
#include <string>

namespace BF
{
size_t encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &key);
size_t decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &key);
} // namespace BF

#endif // BF_H
