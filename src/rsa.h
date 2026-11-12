#ifndef RSA_H
#define RSA_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace rsa
{
using Progress = std::function<bool(size_t done, size_t total)>;
constexpr int CANCELLED = -100;

size_t add_padding(std::vector<uint8_t> &output, std::span<const uint8_t> input);
size_t remove_padding(std::vector<uint8_t> &output, std::span<const uint8_t> input);
int encrypt(std::span<const unsigned char> input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex, const Progress &progress = {});
int decrypt(std::span<const unsigned char> input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex, const Progress &progress = {});
} // namespace rsa

#endif // RSA_H
