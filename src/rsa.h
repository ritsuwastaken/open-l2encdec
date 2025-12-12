#ifndef RSA_H
#define RSA_H

#include <string>
#include <vector>

namespace rsa
{
size_t add_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input);
size_t remove_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input);
void encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex);
int decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex);
} // namespace rsa

#endif // RSA_H
