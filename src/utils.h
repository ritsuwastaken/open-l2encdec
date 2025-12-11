#ifndef UTILS_H
#define UTILS_H
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace utils
{
void add_header(std::vector<unsigned char> &data, std::string_view header);
std::string make_tail(uint32_t crc, size_t crc32_offset, size_t tail_size);
void add_tail(std::vector<unsigned char> &data, std::string_view tail);
} // namespace utils

#endif // UTILS_H
