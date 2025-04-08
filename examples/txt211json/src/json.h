#ifndef JSON_H
#define JSON_H

#include "normalize.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace json
{
    constexpr int INDENT = 4;
    std::string dump(const std::vector<DataBlock> &blocks, int indent = INDENT);
}

#endif // JSON_H
