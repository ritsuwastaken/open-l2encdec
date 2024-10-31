#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <l2encdec.h>

#ifdef _WIN32
#include <getopt.h>
#else
#include <unistd.h>
#endif

std::map<l2encdec::EncodeResult, const char *> ENCODE_ERRORS = {
    {l2encdec::EncodeResult::INVALID_TYPE, "Invalid protocol"},
    {l2encdec::EncodeResult::COMPRESSION_FAILED, "Failed to compress file"},
    {l2encdec::EncodeResult::CRC32_FAILED, "Failed to calculate CRC32"}};

std::map<l2encdec::DecodeResult, const char *> DECODE_ERRORS = {
    {l2encdec::DecodeResult::INVALID_TYPE, "Invalid protocol"},
    {l2encdec::DecodeResult::DECOMPRESSION_FAILED, "Failed to decompress file"},
    {l2encdec::DecodeResult::DECRYPTION_FAILED, "Failed to decrypt file"},
    {l2encdec::DecodeResult::CRC32_FAILED, "Failed to calculate CRC32"},
    {l2encdec::DecodeResult::CRC32_MISMATCH, "CRC32 verification failed"}};

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

std::map<Command, std::string> PREFIXES = {
    {Command::ENCODE, "enc"},
    {Command::DECODE, "dec"}};

const size_t DEFAULT_HEADER_SIZE = 28;

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

void print_usage(const char *name)
{
    std::cout << "Usage:\n"
              << "  " << name << " [-c <command>] [-p <protocol>] [-o <output_file>] [-t] <input_file>\n\n"
              << "Options:\n"
              << "  -h                    print help\n"
              << "  -c <command>          options: encode, decode; default: decode\n"
              << "  -p <protocol>         used for default params, options: 111, 120, 121, 211-212, 411-414\n"
              << "  -o <output_file>      path to output file\n"
              << "  -t                    skip tail verification for decoding; do not add tail when encoding\n"
              << "  -l                    use legacy RSA credentials for decryption; only for protocols 411-414\n"
              << "  -a <algorithm>        possible options: blowfish, rsa, xor, xor_position, xor_filename\n"
              << "  -m <modulus_hex>      custom modulus for `rsa`\n"
              << "  -e/-d <exponent_hex>  custom public or private exponent for `rsa`\n"
              << "  -b <blowfish_key>     custom key for `blowfish`\n"
              << "  -x <xor_key_hex>      custom key for `xor` - protocol 111\n"
              << "  -f <filename>         force different filename for `xor_filename` - protocol 121\n"
              << "  -s <start_index_hex>  custom start index for `xor_position` - protocol 120\n"
              << "  -w <header>           custom wide char header; default: Lineage2Ver<protocol>\n"
              << "  <input_file>          path to input file\n\n"
              << "Example:\n"
              << "  " << name << " -c decode filename.ini\n"
              << "  " << name << " -c encode -p 413 -o enc-filename.ini dec-filename.ini\n"
              << "  " << name << " -c decode -a rsa -m 75b4d6...e2039 -d 1d -o dec-filename.ini -w Lineage2Ver413 filename.ini\n\n"
              << "Source code: " << "https://github.com/ritsuwastaken/open-l2encdec"
              << "\n";
}

int main(int argc, char *argv[])
{
    Command command = Command::DECODE;
    std::string output_filename = "";
    int protocol = 0;
    bool skip_tail = false;
    bool use_legacy_rsa = false;
    l2encdec::Type algorithm = l2encdec::Type::NONE;
    std::string header = "";
    std::string modulus = "";
    std::string exponent = "";
    std::string blowfish_key = "";
    int *xor_key = nullptr;
    int *start_position = nullptr;

    if (argc == 1)
    {
        print_usage(argv[0]);
        std::cin.get();
        return 1;
    }

    std::filesystem::path input_path(argv[optind]);
    std::string input_file = input_path.string();
    std::string input_file_name = input_path.filename().string();
    std::string input_file_dir = input_path.parent_path().string();

    bool has_single_option = argc == 2 && optind == 1;
    bool has_decode_prefix = input_file_name.starts_with(PREFIXES[Command::DECODE] + "-");
    if (has_single_option && has_decode_prefix)
        command = Command::ENCODE;

    int opt;
    while ((opt = getopt(argc, argv, "hc:p:o:tla:w:e:d:m:b:x:s:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(argv[0]);
            return 0;
        case 'c':
            if (!optarg)
            {
                print_usage(argv[0]);
                return 1;
            }

            if (std::strcmp(optarg, "encode") == 0)
                command = Command::ENCODE;
            else if (std::strcmp(optarg, "decode") == 0)
                command = Command::DECODE;
            else
            {
                std::cerr << "Invalid command: " << optarg << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            break;
        case 'p':
            if (!optarg)
            {
                std::cerr << "Protocol option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            try
            {
                protocol = std::stoi(optarg);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Invalid protocol value: " << optarg << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            break;
        case 'o':
            if (!optarg)
            {
                std::cerr << "Output file option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            output_filename = optarg;
            break;
        case 't':
            skip_tail = true;
            break;
        case 'l':
            use_legacy_rsa = true;
            break;
        case 'a':
            if (!optarg || ENCDEC_TYPES.find(optarg) == ENCDEC_TYPES.end())
            {
                std::cerr << "Invalid algorithm" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            algorithm = ENCDEC_TYPES.at(optarg);
            break;
        case 'w':
            if (!optarg)
            {
                std::cerr << "Header option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            header = optarg;
            break;
        case 'e':
        case 'd':
            if (!optarg)
            {
                std::cerr << "Exponent option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            exponent = optarg;
            break;
        case 'm':
            if (!optarg)
            {
                std::cerr << "Modulus option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            modulus = optarg;
            break;
        case 'b':
            if (!optarg)
            {
                std::cerr << "Blowfish key option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            blowfish_key = optarg;
            break;
        case 'x':
            if (!optarg)
            {
                std::cerr << "XOR key option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            try
            {
                xor_key = new int(std::stoi(optarg));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Invalid XOR key value: " << optarg << std::endl;
                return 1;
            }
            break;
        case 's':
            if (!optarg)
            {
                std::cerr << "Start index option requires a value" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            try
            {
                start_position = new int(std::stoi(optarg));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Invalid start index value: " << optarg << std::endl;
                return 1;
            }
            break;
        case '?':
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind >= argc)
    {
        print_usage(argv[0]);
        return 1;
    }

    std::vector<unsigned char> input_data;
    std::vector<unsigned char> output_data;

    if (read(input_file, input_data) != 0)
    {
        std::cerr << "Failed to read input file: " << input_file << std::endl;
        return 1;
    }

    protocol = protocol == 0
                   ? (command == Command::DECODE
                          ? read_protocol_from_input_data(input_data)
                          : read_protocol_from_input_file_name(input_file_name))
                   : protocol;

    l2encdec::Params params;
    if (protocol != 0 && !l2encdec::init_params(&params, protocol, input_file_name, use_legacy_rsa))
        std::cerr << "Warning: unsupported protocol" << std::endl;

    params.skip_tail = skip_tail;
    params.filename = input_file_name;

    if (header != "")
        params.header = header;
    if (algorithm != l2encdec::Type::NONE)
        params.type = algorithm;
    if (modulus != "")
        params.rsa_modulus = modulus;
    if (exponent != "")
    {
        params.rsa_private_exponent = exponent;
        params.rsa_public_exponent = exponent;
    }
    if (blowfish_key != "")
        params.blowfish_key = blowfish_key;
    if (xor_key != nullptr)
        params.xor_key = *xor_key;
    if (start_position != nullptr)
        params.xor_start_position = *start_position;

    std::cout << "Command: " << (command == Command::ENCODE ? "encode" : "decode") << std::endl
              << "Protocol: " << protocol << std::endl;

    switch (command)
    {
    case Command::ENCODE:
        if (auto status = l2encdec::encode(input_data, output_data, params);
            status != l2encdec::EncodeResult::SUCCESS)
        {
            std::cerr << ENCODE_ERRORS.at(status) << std::endl;
            return 1;
        }
        break;
    case Command::DECODE:
        if (auto status = l2encdec::decode(input_data, output_data, params);
            status != l2encdec::DecodeResult::SUCCESS)
        {
            std::cerr << DECODE_ERRORS.at(status) << std::endl;
            return 1;
        }
    }

    if (output_filename == "")
    {
        std::string new_output_file_name = command == Command::ENCODE
                                               ? PREFIXES[command] + "-" + input_file_name
                                               : PREFIXES[command] + "-" + std::to_string(protocol) + "-" + input_file_name;
        output_filename = input_file_dir.empty()
                              ? new_output_file_name
                              : input_file_dir + "/" + new_output_file_name;
    }

    if (write(output_filename, output_data) != 0)
    {
        std::cerr << "Failed to save output file" << std::endl;
        return 1;
    }

    std::cout << "Saved to: " << output_filename << std::endl;

    return 0;
}
