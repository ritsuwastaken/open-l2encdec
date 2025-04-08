#ifndef UTF_H
#define UTF_H

#include <string>
#include <vector>

class utf
{
public:
    static std::string utf16LE_to_utf8(const std::vector<unsigned char> &input_data);

private:
    static constexpr uint8_t UTF16_BOM_FIRST = 0xFF;
    static constexpr uint8_t UTF16_BOM_SECOND = 0xFE;
    static constexpr size_t UTF16_BOM_SIZE = 2;
    static constexpr size_t BYTES_PER_UTF16_CHAR = 2;
};

#endif
