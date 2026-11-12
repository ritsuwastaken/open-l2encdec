#include "blowfish.h"
#include <algorithm>
#include <array>
#include <tomcrypt.h>

namespace
{
constexpr size_t BLOWFISH_BLOCK = 8;
constexpr size_t BLOWFISH_MIN_KEY = 8;
constexpr size_t BLOWFISH_MAX_KEY = 56;

template <int (*Func)(const unsigned char *, unsigned char *, const symmetric_key *)>
size_t process(std::span<const unsigned char> input_data,
               std::vector<unsigned char> &output_data,
               std::string key)
{
    if (key.empty() || key.back() != '\0')
        key.push_back('\0');

    if (key.size() > BLOWFISH_MAX_KEY)
    {
        output_data.clear();
        return size_t(-1);
    }

    const std::string original_key = key;
    while (key.size() < BLOWFISH_MIN_KEY)
        key += original_key;

    symmetric_key bf{};
    if (blowfish_setup(reinterpret_cast<const unsigned char *>(key.data()),
                       static_cast<int>(key.size()), 0, &bf) != CRYPT_OK)
    {
        output_data.clear();
        return size_t(-1);
    }

    output_data.assign(input_data.begin(), input_data.end());
    size_t full_blocks = input_data.size() / BLOWFISH_BLOCK;
    for (size_t i = 0; i < full_blocks * BLOWFISH_BLOCK; i += BLOWFISH_BLOCK)
    {
        std::array<unsigned char, BLOWFISH_BLOCK> block;
        std::reverse_copy(output_data.begin() + i, output_data.begin() + i + 4, block.begin());
        std::reverse_copy(output_data.begin() + i + 4, output_data.begin() + i + 8, block.begin() + 4);

        if (Func(block.data(), block.data(), &bf) != CRYPT_OK)
        {
            output_data.clear();
            return size_t(-1);
        }

        std::reverse_copy(block.begin(), block.begin() + 4, output_data.begin() + i);
        std::reverse_copy(block.begin() + 4, block.end(), output_data.begin() + i + 4);
    }

    blowfish_done(&bf);
    return output_data.size();
}
} // namespace

size_t blowfish::encrypt(std::span<const unsigned char> input_data,
                         std::vector<unsigned char> &output_data,
                         std::string key)
{
    return process<&blowfish_ecb_encrypt>(input_data, output_data, key);
}

size_t blowfish::decrypt(std::span<const unsigned char> input_data,
                         std::vector<unsigned char> &output_data,
                         std::string key)
{
    return process<&blowfish_ecb_decrypt>(input_data, output_data, key);
}
