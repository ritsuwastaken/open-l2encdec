#include <l2encdec.h>
#include <iostream>
#include <vector>
#include <fstream>

inline std::vector<unsigned char> read_file(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

inline void write_file(const std::string &filename, const std::vector<unsigned char> &data)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char *>(data.data()), data.size());
}

int main(int argc, char *argv[])
{
    // Initialize parameters for protocol 120
    l2encdec::Params params;
    if (!l2encdec::init_params(&params, 120))
        return 1;

    // Add custom tail
    params.tail = "00000000000000000000000012e0cdec00000000";

    std::vector<unsigned char> input_data = read_file("input.txt");

    // Encode the data
    std::vector<unsigned char> encoded_data;
    auto encode_result = l2encdec::encode(input_data, encoded_data, params);
    if (encode_result != l2encdec::EncodeResult::SUCCESS)
        return 1;

    write_file("encoded.input.txt", encoded_data);

    // Decode the data
    std::vector<unsigned char> decoded_data;
    auto decode_result = l2encdec::decode(encoded_data, decoded_data, params);
    if (decode_result != l2encdec::DecodeResult::SUCCESS)
        return 1;

    write_file("decoded.input.txt", decoded_data);

    return 0;
}
