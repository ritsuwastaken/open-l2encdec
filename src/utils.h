#ifndef UTILS_H
#define UTILS_H
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace utils
{
void add_header(std::vector<unsigned char> &data, std::string_view header);
void add_tail(std::vector<unsigned char> &data, uint32_t crc, size_t crc32_offset, size_t footer_size);
void add_tail(std::vector<unsigned char> &data, std::string_view tail);
} // namespace utils

#endif // UTILS_H
