#include "blowfish.h"
#include <blowfish/blowfish.h>

static constexpr size_t BLOWFISH_BLOCK = 8;

static inline uint32_t read_u32(const unsigned char *p)
{
    return (uint32_t(p[0])) |
           (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) |
           (uint32_t(p[3]) << 24);
}

static inline void write_u32(uint32_t v, unsigned char *p)
{
    p[0] = (v >> 0) & 0xFF;
    p[1] = (v >> 8) & 0xFF;
    p[2] = (v >> 16) & 0xFF;
    p[3] = (v >> 24) & 0xFF;
}

template <void (Blowfish::*Func)(uint32_t &, uint32_t &)>
size_t process(const std::vector<unsigned char> &input_data,
               std::vector<unsigned char> &output_data,
               std::string key)
{
    output_data = input_data;

    if (key.empty() || key.back() != '\0')
        key.push_back('\0');

    Blowfish bf(key);

    size_t full_blocks = input_data.size() / BLOWFISH_BLOCK;
    for (size_t i = 0; i < full_blocks * BLOWFISH_BLOCK; i += BLOWFISH_BLOCK)
    {
        uint32_t left = read_u32(&output_data[i]);
        uint32_t right = read_u32(&output_data[i + 4]);

        (bf.*Func)(left, right);

        write_u32(left, &output_data[i]);
        write_u32(right, &output_data[i + 4]);
    }

    return output_data.size();
}

size_t blowfish::encrypt(const std::vector<unsigned char> &input_data,
                         std::vector<unsigned char> &output_data,
                         std::string key)
{
    return process<&Blowfish::encrypt>(input_data, output_data, key);
}

size_t blowfish::decrypt(const std::vector<unsigned char> &input_data,
                         std::vector<unsigned char> &output_data,
                         std::string key)
{
    return process<&Blowfish::decrypt>(input_data, output_data, key);
}
