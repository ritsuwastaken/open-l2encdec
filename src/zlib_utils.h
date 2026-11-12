#ifndef ZLIB_UTILS_H
#define ZLIB_UTILS_H

#include <cstdint>
#include <span>
#include <vector>

namespace zlib_utils
{
int unpack(std::span<const unsigned char> input_buffer, std::vector<unsigned char> &output_buffer);
int pack(std::span<const unsigned char> input_buffer, std::vector<unsigned char> &output_buffer);
uint32_t checksum(std::span<const unsigned char> buffer, uint32_t checksum = 0);
} // namespace zlib_utils

#endif // ZLIB_UTILS_H
