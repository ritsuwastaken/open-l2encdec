#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <string>
#include <vector>
#include <unordered_map>

struct DataBlock
{
    std::unordered_map<std::string, std::string> single_values;
    std::unordered_map<std::string, std::vector<std::string>> array_values;
};

std::vector<DataBlock> normalize(const std::string &main_content, const std::string &meta_content);

#endif // NORMALIZE_H
