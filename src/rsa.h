#ifndef RSA_H
#include <iostream>
#include <vector>

namespace RSA
{
    void encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &public_exp_hex);
    int decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &modulus_hex, const std::string &private_exp_hex);
}
#define RSA_H

#endif // RSA_H
