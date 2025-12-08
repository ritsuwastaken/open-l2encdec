#include "utils.h"
#include <cstring>
#include <string>

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

void utils::add_tail(std::vector<unsigned char> &data, uint32_t crc, size_t crc32_offset, size_t footer_size)
{
    const size_t start = data.size();
    data.resize(start + footer_size);
    unsigned char *footer = data.data() + start;
    uint32_t *crc_ptr = reinterpret_cast<uint32_t *>(footer + crc32_offset);
    std::memcpy(crc_ptr, &crc, sizeof(crc));
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
