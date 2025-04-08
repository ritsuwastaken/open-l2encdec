#include "json.h"
#include <nlohmann/json.hpp>

std::string json::dump(const std::vector<DataBlock> &blocks, int indent)
{
    nlohmann::json all = nlohmann::json::array();
    for (const auto &block : blocks)
    {
        nlohmann::json j_block;
        for (const auto &[key, value] : block.single_values)
            j_block[key] = value;

        for (const auto &[key, values] : block.array_values)
            j_block[key] = values;

        all.push_back(j_block);
    }

    return all.dump(indent);
}
