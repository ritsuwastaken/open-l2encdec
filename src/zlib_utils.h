#ifndef ZLIB_UTILS_H
#define ZLIB_UTILS_H

#include <iostream>
#include <vector>
#include <cstdint>

namespace ZlibUtils
{
    int unpack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer);
    int pack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer);
    int checksum(const std::vector<unsigned char> &buffer, uint32_t &checksum, size_t header_size);
}

#endif // ZLIB_UTILS_H
