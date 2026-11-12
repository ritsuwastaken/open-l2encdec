#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace blowfish
{
size_t encrypt(std::span<const unsigned char> input_data, std::vector<unsigned char> &output_data, std::string key);
size_t decrypt(std::span<const unsigned char> input_data, std::vector<unsigned char> &output_data, std::string key);
} // namespace blowfish

#endif // BLOWFISH_H
