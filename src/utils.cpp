#include "utils.h"
#include <charconv>
#include <cstring>
#include <format>

void utils::add_header(std::vector<unsigned char> &data, std::string_view header)
{
    data.insert(data.begin(), header.size() * 2, 0);
    for (size_t i = 0; i < header.size(); i++)
        data[i * 2] = static_cast<unsigned char>(header[i]);
}

std::string utils::make_tail(uint32_t crc, size_t crc32_offset, size_t tail_size)
{
    std::vector<unsigned char> tail(tail_size, 0);
    std::memcpy(tail.data() + crc32_offset, &crc, sizeof(crc));

    std::string result;
    result.reserve(tail_size * 2);

    for (unsigned char b : tail)
        result += std::format("{:02X}", b);

    return result;
}

bool utils::add_tail(std::vector<unsigned char> &data, std::string_view tail)
{
    std::string padded = std::string(tail);
    if (padded.size() % 2 != 0)
        padded.insert(padded.begin(), '0');

    std::vector<unsigned char> bytes;
    bytes.reserve(padded.size() / 2);

    for (size_t i = 0; i < padded.size(); i += 2)
    {
        unsigned char value = 0;
        const char *first = padded.data() + i;
        auto [ptr, ec] = std::from_chars(first, first + 2, value, 16);
        if (ec != std::errc{} || ptr != first + 2)
            return false;
        bytes.push_back(value);
    }

    data.insert(data.end(), bytes.begin(), bytes.end());
    return true;
}
