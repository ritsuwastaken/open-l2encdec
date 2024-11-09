#ifndef ZLIB_UTILS_H
#define ZLIB_UTILS_H

#include <iostream>
#include <vector>
#include <cstdint>

namespace ZlibUtils
{
    int unpack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer);
    int pack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer);
    uint32_t checksum(const std::vector<unsigned char> &buffer, uint32_t checksum = 0);
}

#endif // ZLIB_UTILS_H
