#include "bf.h"
#include <blowfish.h>

size_t BF::encrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const unsigned char *key, int key_size)
{
    Blowfish blowfish;
    blowfish.SetKey(key, key_size);
    output_data.resize(input_data.size());
    blowfish.Encrypt(output_data.data(), input_data.data(), static_cast<int>(input_data.size()));
    return output_data.size();
}

size_t BF::decrypt(const std::vector<unsigned char> &input_data, std::vector<unsigned char> &output_data, const unsigned char *key, int key_size)
{
    Blowfish blowfish;
    blowfish.SetKey(key, key_size);
    output_data.resize(input_data.size());
    blowfish.Decrypt(output_data.data(), input_data.data(), static_cast<int>(input_data.size()));
    return output_data.size();
}
