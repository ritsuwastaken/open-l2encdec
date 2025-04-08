#include "normalize.h"
#include <sstream>

namespace
{
    constexpr const char *ITEM_TYPE = "item";
    constexpr const char *SKILL_TYPE = "skill";

    constexpr const char *OBJECT_ID_FIELD = "object_id";
    constexpr const char *SKILL_ID_FIELD = "skill_id";
    constexpr const char *ID_FIELD = "id";
    constexpr const char *UNDERSCORE_NAME_BEGIN_FIELD = "_name_begin";
    constexpr const char *UNDERSCORE_NAME_END_FIELD = "_name_end";
    constexpr const char *UNDERSCORE_BEGIN_FIELD = "_begin";
    constexpr const char *UNDERSCORE_END_FIELD = "_end";

    std::vector<std::string> split(const std::string &s, char delim)
    {
        std::vector<std::string> out;
        std::string temp;
        for (char ch : s)
        {
            if (ch == delim)
            {
                if (!temp.empty())
                    out.push_back(temp);
                temp.clear();
            }
            else
                temp += ch;
        }
        if (!temp.empty())
            out.push_back(temp);
        return out;
    }

    std::string trim(const std::string &s)
    {
        auto start = s.begin();
        while (start != s.end() && std::isspace(*start))
            ++start;

        auto end = s.end();
        while (end != start && std::isspace(*(end - 1)))
            --end;

        return std::string(start, end);
    }

    std::string clean_brackets(const std::string &s)
    {
        if (!s.empty() && s.front() == '[' && s.back() == ']')
            return s.substr(1, s.size() - 2);
        return s;
    }

    bool has_block_tags(const std::string &line, const std::string &block_type, const std::string &begin_suffix, const std::string &end_suffix)
    {
        std::string start_tag = block_type + begin_suffix;
        std::string end_tag = block_type + end_suffix;
        return line.find(start_tag) != std::string::npos && line.find(end_tag) != std::string::npos;
    }

    bool is_main_block(const std::string &line, const std::string &block_type)
    {
        return has_block_tags(line, block_type, UNDERSCORE_BEGIN_FIELD, UNDERSCORE_END_FIELD);
    }

    bool is_meta_block(const std::string &line, const std::string &block_type)
    {
        return has_block_tags(line, block_type, UNDERSCORE_NAME_BEGIN_FIELD, UNDERSCORE_NAME_END_FIELD);
    }

    std::string get_id_from_block(const DataBlock &block)
    {
        if (auto it = block.single_values.find(OBJECT_ID_FIELD); it != block.single_values.end())
            return it->second;
        if (auto it = block.single_values.find(SKILL_ID_FIELD); it != block.single_values.end())
            return it->second;
        return "";
    }

    std::vector<std::string> parse_braced_values(const std::string &val)
    {
        std::string inner = val.substr(1, val.size() - 2);
        auto parts = inner.find(';') != std::string::npos ? split(inner, ';') : std::vector<std::string>{inner};

        std::vector<std::string> result;
        for (const auto &p : parts)
            result.push_back(clean_brackets(trim(p)));
        return result;
    }

    DataBlock parse_line(const std::string &line)
    {
        DataBlock result;
        auto tokens = split(line, '\t');

        for (const auto &tok : tokens)
        {
            if (tok.ends_with(UNDERSCORE_BEGIN_FIELD) || tok.ends_with(UNDERSCORE_END_FIELD))
                continue;

            auto eq = tok.find('=');
            if (eq == std::string::npos)
                continue;

            auto key = trim(tok.substr(0, eq));
            auto val = trim(tok.substr(eq + 1));

            if (val.front() == '{' && val.back() == '}')
            {
                result.array_values[key] = parse_braced_values(val);
            }
            else
            {
                result.single_values[key] = clean_brackets(val);
            }
        }

        return result;
    }

    void merge_metadata(DataBlock &block, const DataBlock &meta)
    {
        for (const auto &[key, value] : meta.single_values)
        {
            if (key != ID_FIELD && !block.single_values.contains(key))
                block.single_values[key] = value;
        }

        for (const auto &[key, value] : meta.array_values)
        {
            if (key != ID_FIELD && !block.array_values.contains(key))
                block.array_values[key] = value;
        }
    }

    std::string detect_block_type(const std::string &content)
    {
        std::istringstream in(content);
        std::string line;

        while (std::getline(in, line))
        {
            if (is_main_block(line, ITEM_TYPE))
                return ITEM_TYPE;
            if (is_main_block(line, SKILL_TYPE))
                return SKILL_TYPE;
        }

        throw std::runtime_error("Unsupported or undetected block type. Only 'item' and 'skill' are supported.");
    }

    using MetaMap = std::unordered_map<std::string, DataBlock>;

    MetaMap load_metadata(const std::string &content, const std::string &block_type)
    {
        std::istringstream in(content);
        std::string line;
        MetaMap map;

        while (std::getline(in, line))
        {
            if (is_meta_block(line, block_type))
            {
                DataBlock block = parse_line(line);
                std::string id_key = block.single_values.contains(SKILL_ID_FIELD) ? SKILL_ID_FIELD : ID_FIELD;
                map[block.single_values[id_key]] = block;
            }
        }

        return map;
    }
}

std::vector<DataBlock> normalize(const std::string &main_content, const std::string &meta_content)
{
    std::string block_type = detect_block_type(main_content);
    MetaMap meta = load_metadata(meta_content, block_type);

    std::vector<DataBlock> all_blocks;
    std::istringstream in(main_content);
    std::string line;

    while (std::getline(in, line))
    {
        if (is_main_block(line, block_type))
        {
            DataBlock block = parse_line(line);
            std::string id = get_id_from_block(block);

            if (!id.empty() && meta.count(id))
            {
                merge_metadata(block, meta[id]);
            }

            all_blocks.push_back(std::move(block));
        }
    }

    return all_blocks;
}
