#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <cstddef>
#include <string>
#include <vector>

namespace blowfish
{
size_t encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &key);
size_t decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &key);
} // namespace blowfish

#endif // BLOWFISH_H
