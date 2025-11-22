#include "bf.h"
#include <blowfish.h>

size_t BF::encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &key)
{
    Blowfish blowfish;
    blowfish.SetKey(reinterpret_cast<const unsigned char*>(key.data()), key.size() + 1);
    output_data.resize(input_data.size());
    blowfish.Encrypt(output_data.data(), input_data.data(), static_cast<int>(input_data.size()));
    return output_data.size();
}

size_t BF::decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const std::string &key)
{
    Blowfish blowfish;
    blowfish.SetKey(reinterpret_cast<const unsigned char*>(key.data()), key.size() + 1);
    output_data.resize(input_data.size());
    blowfish.Decrypt(output_data.data(), input_data.data(), static_cast<int>(input_data.size()));
    return output_data.size();
}
