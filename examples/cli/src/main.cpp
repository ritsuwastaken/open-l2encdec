#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <l2encdec.h>
#include "options.h"

#ifdef _WIN32
#include <getopt.h>
#else
#include <unistd.h>
#endif

std::map<l2encdec::EncodeResult, const char *> ENCODE_ERRORS = {
    {l2encdec::EncodeResult::INVALID_TYPE, "Invalid protocol"},
    {l2encdec::EncodeResult::COMPRESSION_FAILED, "Failed to compress file"}};

std::map<l2encdec::DecodeResult, const char *> DECODE_ERRORS = {
    {l2encdec::DecodeResult::INVALID_TYPE, "Invalid protocol"},
    {l2encdec::DecodeResult::DECOMPRESSION_FAILED, "Failed to decompress file"},
    {l2encdec::DecodeResult::DECRYPTION_FAILED, "Failed to decrypt file"}};

std::map<l2encdec::ChecksumResult, const char *> CHECKSUM_ERRORS = {
    {l2encdec::ChecksumResult::MISMATCH, "Checksum mismatch"}};

std::map<std::string, l2encdec::Type> ENCDEC_TYPES = {
    {"blowfish", l2encdec::Type::BLOWFISH},
    {"rsa", l2encdec::Type::RSA},
    {"xor", l2encdec::Type::XOR},
    {"xor_position", l2encdec::Type::XOR_POSITION},
    {"xor_filename", l2encdec::Type::XOR_FILENAME}};

enum class Command
{
    ENCODE,
    DECODE
};

std::map<std::string, Command> COMMANDS = {
    {"encode", Command::ENCODE},
    {"decode", Command::DECODE}};

std::map<Command, std::string> PREFIXES = {
    {Command::ENCODE, "enc"},
    {Command::DECODE, "dec"}};

const size_t DEFAULT_HEADER_SIZE = 28;
const size_t TAIL_HEX_SIZE = 40;

int read(const std::string &filename, std::vector<unsigned char> &data)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        return 1;

    data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return 0;
}

int write(const std::string &filename, const std::vector<unsigned char> &data)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
        return 1;

    file.write(reinterpret_cast<const char *>(data.data()), data.size());
    if (file.bad())
        return 1;

    return 0;
}

int read_protocol_from_input_data(std::vector<unsigned char> &data)
{
    if (data.size() < DEFAULT_HEADER_SIZE)
        return 0;

    std::string string;
    for (size_t i = 0; i < DEFAULT_HEADER_SIZE; i += 2)
    {
        if (data[i + 1] == 0)
        {
            string += static_cast<char>(data[i]);
        }
    }

    try
    {
        int protocol = std::stoi(string.substr(11));
        if (std::find(std::begin(l2encdec::SUPPORTED_PROTOCOLS), std::end(l2encdec::SUPPORTED_PROTOCOLS), protocol) != std::end(l2encdec::SUPPORTED_PROTOCOLS))
            return protocol;
        return 0;
    }
    catch (...)
    {
        return 0;
    }
}

int read_protocol_from_input_file_name(const std::string &input_file_name)
{
    try
    {
        return std::stoi(input_file_name.substr(4, 3));
    }
    catch (...)
    {
        return 0;
    }
}

int main(int argc, char *argv[])
{
    auto options = options::parse(argc, argv);
    std::vector<unsigned char> input_data;
    std::vector<unsigned char> output_data;

    if (read(options.input_file, input_data) != 0)
    {
        std::cerr << "Failed to read input file: " << options.input_file << std::endl;
        return 1;
    }

    std::cout << "Command: " << (options.is_encode ? "encode" : "decode") << std::endl;

    if (options.is_encode)
    {
        if (auto status = l2encdec::encode(input_data, output_data, options.params);
            status != l2encdec::EncodeResult::SUCCESS)
        {
            std::cerr << ENCODE_ERRORS.at(status) << std::endl;
            return 1;
        }
    }
    else
    {
        if (options.verify && !options.params.skip_tail)
        {
            if (auto status = l2encdec::verify_checksum(input_data);
                status != l2encdec::ChecksumResult::SUCCESS)
            {
                std::cerr << CHECKSUM_ERRORS.at(status) << std::endl;
                return 1;
            }
        }
        if (auto status = l2encdec::decode(input_data, output_data, options.params);
            status != l2encdec::DecodeResult::SUCCESS)
        {
            std::cerr << DECODE_ERRORS.at(status) << std::endl;
            return 1;
        }
    }

    if (write(options.output_filename, output_data) != 0)
    {
        std::cerr << "Failed to save output file" << std::endl;
        return 1;
    }

    std::cout << "Saved to: " << options.output_filename << std::endl;
    return 0;
}
