#include "json.h"
#include "normalize.h"
#include "utf.h"
#include <fstream>
#include <iostream>
#include <l2encdec.h>
#include <string>
#include <vector>

namespace
{
constexpr int L2_PROTOCOL_VERSION = 211;
const std::string DEFAULT_OUTPUT_FILENAME = "output.json";
} // namespace

inline std::vector<unsigned char> read_file(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: ./txt211json <main_file> <meta_file> [output_file]\n";
        return 1;
    }

    try
    {
        const std::string output_filename = (argc > 3) ? argv[3] : DEFAULT_OUTPUT_FILENAME;

        auto encrypted_main_content = read_file(argv[1]);
        auto encrypted_meta_content = read_file(argv[2]);

        std::vector<unsigned char> main_content;
        if (l2encdec::decode(encrypted_main_content, main_content, L2_PROTOCOL_VERSION) != l2encdec::DecodeResult::SUCCESS)
            throw std::runtime_error("Failed to decode main file: " + std::string(argv[1]));

        std::vector<unsigned char> meta_content;
        if (l2encdec::decode(encrypted_meta_content, meta_content, L2_PROTOCOL_VERSION) != l2encdec::DecodeResult::SUCCESS)
            throw std::runtime_error("Failed to decode meta file: " + std::string(argv[2]));

        auto normalized = normalize(
            utf::utf16LE_to_utf8(main_content),
            utf::utf16LE_to_utf8(meta_content));

        std::ofstream file(output_filename);
        file << json::dump(normalized);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
