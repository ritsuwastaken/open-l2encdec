#include "options.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>

#ifdef _WIN32
#include <getopt.h>
#else
#include <unistd.h>
#endif

namespace
{
    const size_t DEFAULT_HEADER_SIZE = 28;
    const size_t TAIL_HEX_SIZE = 40;

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

    std::map<std::string, l2encdec::Type> ENCDEC_TYPES = {
        {"blowfish", l2encdec::Type::BLOWFISH},
        {"rsa", l2encdec::Type::RSA},
        {"xor", l2encdec::Type::XOR},
        {"xor_position", l2encdec::Type::XOR_POSITION},
        {"xor_filename", l2encdec::Type::XOR_FILENAME}};

    int read_protocol_from_input_data(const std::vector<unsigned char> &data)
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
              << "  -v                    verify checksum before decoding\n"
              << "  -t                    do not add tail/read file without tail (e.g. for Exteel files)\n"
              << "  -l                    use legacy RSA credentials for decryption; only for protocols 411-414\n"
              << "  -a <algorithm>        possible options: blowfish, rsa, xor, xor_position, xor_filename\n"
              << "  -m <modulus_hex>      custom modulus for `rsa`\n"
              << "  -e/-d <exponent_hex>  custom public or private exponent for `rsa`\n"
              << "  -b <blowfish_key>     custom key for `blowfish`\n"
              << "  -x <xor_key_hex>      custom key for `xor` - protocol 111\n"
              << "  -f <filename>         force different filename for `xor_filename` - protocol 121\n"
              << "  -s <start_index_hex>  custom start index for `xor_position` - protocol 120\n"
              << "  -w <header>           custom wide char header; default: Lineage2Ver<protocol>\n"
              << "  -T <tail_hex>         custom tail for encoding, e.g. 000000000000000000000000deadbeaf00000000; contains checksum by default\n"
              << "  <input_file>          path to input file\n\n"
              << "Example:\n"
              << "  " << name << " -c decode filename.ini\n"
              << "  " << name << " -c encode -p 413 -o enc-filename.ini dec-filename.ini\n"
              << "  " << name << " -c decode -a rsa -m 75b4d6...e2039 -d 1d -o dec-filename.ini -w Lineage2Ver413 filename.ini\n\n"
              << "Source code: " << "https://github.com/ritsuwastaken/open-l2encdec"
              << "\n";
}

Options options::parse(int argc, char *argv[])
{
    Options options;
    options.is_encode = false;
    options.verify = false;
    options.algorithm = l2encdec::Type::NONE;

    int protocol = 0;
    bool use_legacy_decrypt_rsa = false;
    std::string header = "";
    std::string tail = "";
    std::string modulus = "";
    std::string exponent = "";
    std::string blowfish_key = "";
    std::string filename = "";
    int *xor_key = nullptr;
    int *xor_start_position = nullptr;

    if (argc == 1)
    {
        print_usage(argv[0]);
        std::exit(1);
    }

    bool has_single_option = argc == 2 && optind == 1;
    if (has_single_option)
    {
        std::filesystem::path input_path(argv[optind]);
        bool has_decode_prefix = input_path.filename().string().starts_with(PREFIXES[Command::DECODE] + "-");
        if (has_decode_prefix)
            options.is_encode = true;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hc:p:o:tla:w:e:d:m:b:x:s:vfT:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(argv[0]);
            std::exit(0);
        case 'c':
            if (!optarg || !COMMANDS.contains(optarg))
            {
                print_usage(argv[0]);
                std::exit(1);
            }
            options.is_encode = COMMANDS.at(optarg) == Command::ENCODE;
            break;
        case 'p':
            if (!optarg)
            {
                std::cerr << "Protocol option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            try
            {
                protocol = std::stoi(optarg);
            }
            catch (const std::exception &)
            {
                std::cerr << "Invalid protocol value: " << optarg << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            break;
        case 'o':
            if (!optarg)
            {
                std::cerr << "Output file option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            options.output_filename = optarg;
            break;
        case 't':
            options.params.skip_tail = true;
            break;
        case 'l':
            use_legacy_decrypt_rsa = true;
            break;
        case 'a':
            if (!optarg || !ENCDEC_TYPES.contains(optarg))
            {
                std::cerr << "Invalid algorithm" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            options.algorithm = ENCDEC_TYPES.at(optarg);
            break;
        case 'w':
            if (!optarg)
            {
                std::cerr << "Header option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            header = optarg;
            break;
        case 'e':
        case 'd':
            if (!optarg)
            {
                std::cerr << "Exponent option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            exponent = optarg;
            break;
        case 'm':
            if (!optarg)
            {
                std::cerr << "Modulus option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            modulus = optarg;
            break;
        case 'b':
            if (!optarg)
            {
                std::cerr << "Blowfish key option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            blowfish_key = optarg;
            break;
        case 'x':
            if (!optarg)
            {
                std::cerr << "XOR key option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            try
            {
                xor_key = new int(std::stoi(optarg));
            }
            catch (const std::exception &)
            {
                std::cerr << "Invalid XOR key value: " << optarg << std::endl;
                std::exit(1);
            }
            break;
        case 's':
            if (!optarg)
            {
                std::cerr << "Start index option requires a value" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            try
            {
                xor_start_position = new int(std::stoi(optarg));
            }
            catch (const std::exception &)
            {
                std::cerr << "Invalid start index value: " << optarg << std::endl;
                std::exit(1);
            }
            break;
        case 'v':
            options.verify = true;
            break;
        case 'f':
            if (!optarg)
            {
                std::cerr << "Missing filename for -f option" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            filename = optarg;
            break;
        case 'T':
            if (!optarg)
            {
                std::cerr << "Missing hex string for -T option" << std::endl;
                print_usage(argv[0]);
                std::exit(1);
            }
            tail = optarg;
            if (tail.size() != TAIL_HEX_SIZE)
            {
                std::cerr << "Tail hex string must be exactly 40 symbols (20 bytes)" << std::endl;
                std::exit(1);
            }
            break;
        case '?':
            print_usage(argv[0]);
            std::exit(1);
        }
    }

    if (optind >= argc)
    {
        print_usage(argv[0]);
        std::exit(1);
    }

    std::filesystem::path input_path(argv[optind]);
    options.input_file = input_path.string();
    std::string input_file_name = input_path.filename().string();
    std::string input_file_dir = input_path.parent_path().string();

    // std::vector<unsigned char> input_data;
    // if (read(options.input_file, input_data) != 0)
    // {
    //     std::cerr << "Failed to read input file: " << options.input_file << std::endl;
    //     std::exit(1);
    // }

    // protocol = protocol == 0
    //                ? (!options.is_encode
    //                       ? read_protocol_from_input_data(input_data)
    //                       : read_protocol_from_input_file_name(input_file_name))
    //                : protocol;

    if (protocol != 0 && !l2encdec::init_params(&options.params, protocol, input_file_name, use_legacy_decrypt_rsa))
        std::cerr << "Warning: unsupported protocol" << std::endl;

    options.params.filename = filename == "" ? input_file_name : filename;

    if (header != "")
        options.params.header = header;
    if (tail != "")
        options.params.tail = tail;
    if (options.algorithm != l2encdec::Type::NONE)
        options.params.type = options.algorithm;
    if (modulus != "")
        options.params.rsa_modulus = modulus;
    if (exponent != "")
    {
        options.params.rsa_private_exponent = exponent;
        options.params.rsa_public_exponent = exponent;
    }
    if (blowfish_key != "")
        options.params.blowfish_key = blowfish_key;
    if (xor_key != nullptr)
        options.params.xor_key = *xor_key;
    if (xor_start_position != nullptr)
        options.params.xor_start_position = *xor_start_position;

    if (options.output_filename == "")
    {
        std::string new_output_file_name = options.is_encode
                                               ? PREFIXES[Command::ENCODE] + "-" + input_file_name
                                               : PREFIXES[Command::DECODE] + "-" + std::to_string(protocol) + "-" + input_file_name;
        options.output_filename = input_file_dir.empty()
                                      ? new_output_file_name
                                      : input_file_dir + "/" + new_output_file_name;
    }

    delete xor_key;
    delete xor_start_position;

    return options;
}
