#include "xor_utils.h"
#include <algorithm>
#include <cctype>

namespace
{
int get_key_by_index(int index)
{
    int d1 = index & 0xf;
    int d2 = (index >> 4) & 0xf;
    int d3 = (index >> 8) & 0xf;
    int d4 = (index >> 12) & 0xf;
    return ((d2 ^ d4) << 4) | (d1 ^ d3);
}

int get_key_by_filename(std::string filename)
{
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    int acc = 0;
    for (char c : filename)
        acc += static_cast<int>(c);
    return acc & 0xff;
}
} // namespace

size_t xor_utils::apply(const std::vector<unsigned char> &input,
                        std::vector<unsigned char> &output,
                        const Params &params)
{
    output = input;
    if (params.start_position.has_value())
    {
        int index = *params.start_position;
        for (auto &byte : output)
            byte ^= static_cast<unsigned char>(get_key_by_index(index++));
        return output.size();
    }

    int key = params.filename.empty() ? params.key : get_key_by_filename(params.filename);
    for (auto &byte : output)
        byte ^= static_cast<unsigned char>(key);
    return output.size();
}
