#include "bf.h"
#include <blowfish/blowfish.h>

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

size_t BF::encrypt(const std::vector<unsigned char> &input_data,
                   std::vector<unsigned char> &output_data,
                   const std::string &key)
{
    Blowfish blowfish;
    blowfish.initialize(
        reinterpret_cast<const unsigned char *>(key.data()),
        key.size() + 1);

    size_t len = input_data.size();
    size_t block = 8;

    output_data = input_data;

    size_t full_blocks = len / block;

    for (size_t i = 0; i < full_blocks * block; i += block)
    {
        uint32_t left = read_u32(&output_data[i]);
        uint32_t right = read_u32(&output_data[i + 4]);

        blowfish.encrypt(left, right);

        write_u32(left, &output_data[i]);
        write_u32(right, &output_data[i + 4]);
    }

    return output_data.size();
}

size_t BF::decrypt(const std::vector<unsigned char> &input_data,
                   std::vector<unsigned char> &output_data,
                   const std::string &key)
{
    Blowfish blowfish;
    blowfish.initialize(
        reinterpret_cast<const unsigned char *>(key.data()),
        key.size() + 1);

    size_t len = input_data.size();
    size_t block = 8;

    output_data = input_data;

    size_t full_blocks = len / block;

    for (size_t i = 0; i < full_blocks * block; i += block)
    {
        uint32_t left = read_u32(&input_data[i]);
        uint32_t right = read_u32(&input_data[i + 4]);

        blowfish.decrypt(left, right);

        write_u32(left, &output_data[i]);
        write_u32(right, &output_data[i + 4]);
    }

    return output_data.size();
}
