#include "utf.h"
#include <utf8.h>
#include <iostream>

std::string utf::utf16LE_to_utf8(const std::vector<unsigned char> &input_data)
{
    std::string utf8_result;
    try
    {
        size_t start_pos = 0;
        if (input_data.size() >= UTF16_BOM_SIZE &&
            input_data[0] == UTF16_BOM_FIRST &&
            input_data[1] == UTF16_BOM_SECOND)
        {
            start_pos = UTF16_BOM_SIZE;
        }

        std::vector<unsigned char> output_data = input_data;
        if ((output_data.size() - start_pos) % 2 != 0)
        {
            output_data.push_back(0);
        }

        std::vector<uint16_t> utf16_data;
        utf16_data.reserve((output_data.size() - start_pos) / 2);

        for (size_t i = start_pos; i < output_data.size(); i += 2)
        {
            uint16_t code_unit = static_cast<uint16_t>(output_data[i]) |
                                 (static_cast<uint16_t>(output_data[i + 1]) << 8);
            utf16_data.push_back(code_unit);
        }

        utf8::utf16to8(utf16_data.begin(), utf16_data.end(), std::back_inserter(utf8_result));
    }
    catch (const utf8::exception &e)
    {
        std::cerr << "UTF conversion error: " << e.what() << std::endl;
        return std::string(input_data.begin(), input_data.end());
    }

    return utf8_result;
}
