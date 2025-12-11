#include "utils.h"
#include <cstring>
#include <format>

void utils::add_header(std::vector<unsigned char> &data, std::string_view header)
{
    const size_t wide_size = header.size() * 2;
    data.reserve(data.size() + wide_size);

    std::vector<unsigned char> wide;
    wide.resize(wide_size);

    for (size_t i = 0; i < header.size(); i++)
    {
        wide[i * 2] = static_cast<unsigned char>(header[i]);
        wide[i * 2 + 1] = 0;
    }

    data.insert(data.begin(), wide.begin(), wide.end());
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

void utils::add_tail(std::vector<unsigned char> &data, std::string_view tail)
{
    std::string padded = std::string(tail);
    if (padded.size() % 2 != 0)
        padded.insert(padded.begin(), '0');

    const size_t byte_count = padded.size() / 2;
    data.reserve(data.size() + byte_count);

    for (size_t i = 0; i < padded.size(); i += 2)
    {
        unsigned char value = static_cast<unsigned char>(std::stoi(padded.substr(i, 2), nullptr, 16));
        data.push_back(value);
    }
}
