#ifndef XOR_UTILS_H
#define XOR_UTILS_H

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace xor_utils
{
struct Params
{
    int key = 0;
    std::string filename;
    std::optional<int> start_position;
};

size_t apply(std::span<const unsigned char> input, std::vector<unsigned char> &output, const Params &params);
} // namespace xor_utils

#endif // XOR_UTILS_H
