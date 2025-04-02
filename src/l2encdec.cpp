#include "l2encdec_private.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include "xor.h"
#include "bf.h"
#include "rsa.h"
#include "zlib_utils.h"

const size_t FOOTER_SIZE = 20;
const size_t FOOTER_CRC32_OFFSET = 12;
const int BLOWFISH_KEY_NULL_TERMINATOR = 1;

const std::unordered_map<int, l2encdec::Params> PROTOCOL_CONFIGS = {
    {111, {.type = l2encdec::Type::XOR, .xor_key = 0xAC}},
    {120, {.type = l2encdec::Type::XOR_POSITION, .xor_start_position = 0xE6}},
    {121, {.type = l2encdec::Type::XOR_FILENAME}},
    {211, {.type = l2encdec::Type::BLOWFISH, .blowfish_key = "31==-%&@!^+][;'.]94-"}},
    {212, {.type = l2encdec::Type::BLOWFISH, .blowfish_key = "[;'.]94-&@%!^+]-31=="}}};

struct RSAConfig
{
    std::string modulus;
    std::string private_exponent;
    std::string public_exponent = "";
};

const std::unordered_map<int, RSAConfig> RSA_CONFIGS = {
    {411, {"8c9d5da87b30f5d7cd9dc88c746eaac5bb180267fa11737358c4c95d9adf59dd37689f9befb251508759555d6fe0eca87bebe0a10712cf0ec245af84cd22eb4cb675e98eaf5799fca62a20a2baa4801d5d70718dcd43283b8428f1387aec6600f937bfc7bb72404d187d3a9c438f1ffce9ce365dccf754232ff6def038a41385", "1d"}},
    {412, {"a465134799cf2c45087093e7d0f0f144e6d528110c08f674730d436e40827330eccea46e70acf10cdda7d8f710e3b44dcca931812d76cd7494289bca8b73823f57efc0515b97e4a2a02612ccfa719cf7885104b06f2e7e2cc967b62e3d3b1aadb925db94cbc8cd3070a4bb13f7e202c7733a67b1b94c1ebc0afcbe1a63b448cf", "25"}},
    {413, {"97df398472ddf737ef0a0cd17e8d172f0fef1661a38a8ae1d6e829bc1c6e4c3cfc19292dda9ef90175e46e7394a18850b6417d03be6eea274d3ed1dde5b5d7bde72cc0a0b71d03608655633881793a02c9a67d9ef2b45eb7c08d4be329083ce450e68f7867b6749314d40511d09bc5744551baa86a89dc38123dc1668fd72d83", "35"}},
    {414, {"ad70257b2316ce09dfaf2ebc3f63b3d673b0c98a403950e26bb87379b11e17aed0e45af23e7171e5ec1fbc8d1ae32ffb7801b31266eef9c334b53469d4b7cbe83284273d35a9aab49b453e7012f374496c65f8089f5d134b0eb3d1e3b22051ed5977a6dd68c4f85785dfcc9f4412c81681944fc4b8ce27caf0242deaa5762e8d", "25"}}};

const RSAConfig MODERN_RSA_CONFIG = {
    "75b4d6de5c016544068a1acf125869f43d2e09fc55b8b1e289556daf9b8757635593446288b3653da1ce91c87bb1a5c18f16323495c55d7d72c0890a83f69bfd1fd9434eb1c02f3e4679edfa43309319070129c267c85604d87bb65bae205de3707af1d2108881abb567c3b3d069ae67c3a4c6a3aa93d26413d4c66094ae2039",
    "1d",
    "30b4c2d798d47086145c75063c8e841e719776e400291d7838d3e6c4405b504c6a07f8fca27f32b86643d2649d1d5f124cdd0bf272f0909dd7352fe10a77b34d831043d9ae541f8263c6fe3d1c14c2f04e43a7253a6dda9a8c1562cbd493c1b631a1957618ad5dfe5ca28553f746e2fc6f2db816c7db223ec91e955081c1de65",
};

inline void insert_header(std::vector<unsigned char> &data, std::string header)
{
    std::vector<unsigned char> wide_header_bytes;
    for (char c : header)
    {
        wide_header_bytes.push_back(c);
        wide_header_bytes.push_back(0x00);
    }
    data.insert(data.begin(), wide_header_bytes.begin(), wide_header_bytes.end());
}

inline void insert_tail(std::vector<unsigned char> &data, int crc)
{
    data.insert(data.end(), FOOTER_SIZE, 0x00);
    *reinterpret_cast<uint32_t *>(data.data() + data.size() - FOOTER_SIZE + FOOTER_CRC32_OFFSET) = crc;
}

inline void insert_tail(std::vector<unsigned char> &data, const std::string &tail)
{
    std::string padded_tail = tail;
    if (padded_tail.size() % 2 != 0)
        padded_tail = "0" + padded_tail;

    std::vector<unsigned char> tail_bytes;
    for (size_t i = 0; i < padded_tail.size(); i += 2)
    {
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << padded_tail.substr(i, 2);
        ss >> byte;
        tail_bytes.push_back(static_cast<unsigned char>(byte));
    }

    data.insert(data.end(), tail_bytes.begin(), tail_bytes.end());
}

inline int get_XOR_key_by_index(int index)
{
    int d1 = index & 0xf;
    int d2 = (index >> 4) & 0xf;
    int d3 = (index >> 8) & 0xf;
    int d4 = (index >> 12) & 0xf;
    return ((d2 ^ d4) << 4) | (d1 ^ d3);
}

inline int get_XOR_key_by_filename(std::string filename)
{
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    int ind = 0;
    for (char c : filename)
        ind += static_cast<int>(c);
    return ind & 0xff;
}

L2ENCDEC_API bool l2encdec::init_params(Params *params, int protocol, std::string filename, bool use_legacy_decrypt_rsa)
{
    if (auto it = PROTOCOL_CONFIGS.find(protocol); it != PROTOCOL_CONFIGS.end())
    {
        *params = it->second;
    }
    else if (protocol >= 411 && protocol <= 414)
    {
        params->type = l2encdec::Type::RSA;
        auto rsa_config = use_legacy_decrypt_rsa ? RSA_CONFIGS.at(protocol) : MODERN_RSA_CONFIG;
        params->rsa_modulus = rsa_config.modulus;
        params->rsa_private_exponent = rsa_config.private_exponent;
        params->rsa_public_exponent = rsa_config.public_exponent;
    }
    else
    {
        return false;
    }

    if (protocol == 121 && filename.empty())
        return false;

    params->filename = filename;
    params->header = "Lineage2Ver" + std::to_string(protocol);

    return true;
}

L2ENCDEC_API l2encdec::ChecksumResult l2encdec::verify_checksum(const std::vector<unsigned char> &input_data)
{
    uint32_t checksum = *reinterpret_cast<const uint32_t *>(input_data.data() + input_data.size() - FOOTER_SIZE + FOOTER_CRC32_OFFSET);
    std::vector<unsigned char> input_data_without_footer(input_data.begin(), input_data.end() - FOOTER_SIZE);
    uint32_t calculated_checksum = ZlibUtils::checksum(input_data_without_footer);
    return calculated_checksum == checksum ? l2encdec::ChecksumResult::SUCCESS : l2encdec::ChecksumResult::MISMATCH;
}

L2ENCDEC_API l2encdec::EncodeResult l2encdec::encode(const std::vector<unsigned char> &input_data,
                                                     std::vector<unsigned char> &output_data,
                                                     const Params &params)
{
    std::vector<unsigned char> encrypted_data;
    switch (params.type)
    {
    case l2encdec::Type::XOR:
        XOR::encrypt(input_data, encrypted_data, params.xor_key);
        break;
    case l2encdec::Type::XOR_FILENAME:
        XOR::encrypt(input_data, encrypted_data, get_XOR_key_by_filename(params.filename));
        break;
    case l2encdec::Type::XOR_POSITION:
        XOR::encrypt(input_data, encrypted_data, params.xor_start_position, get_XOR_key_by_index);
        break;
    case l2encdec::Type::BLOWFISH:
    {
        int key_size = static_cast<int>(params.blowfish_key.length()) + BLOWFISH_KEY_NULL_TERMINATOR;
        BF::encrypt(input_data, encrypted_data, reinterpret_cast<const unsigned char *>(params.blowfish_key.c_str()), key_size);
        break;
    }
    case l2encdec::Type::RSA:
    {
        std::vector<unsigned char> compressed_data;
        if (ZlibUtils::pack(input_data, compressed_data) != 0)
            return l2encdec::EncodeResult::COMPRESSION_FAILED;
        RSA::encrypt(compressed_data, encrypted_data, params.rsa_modulus, params.rsa_public_exponent);
        break;
    }
    case l2encdec::Type::NONE:
        encrypted_data = input_data;
        break;
    }

    insert_header(encrypted_data, params.header);
    if (!params.skip_tail)
        params.tail.empty()
            ? insert_tail(encrypted_data, ZlibUtils::checksum(encrypted_data))
            : insert_tail(encrypted_data, params.tail);

    output_data = std::move(encrypted_data);

    return l2encdec::EncodeResult::SUCCESS;
}

L2ENCDEC_API l2encdec::DecodeResult l2encdec::decode(const std::vector<unsigned char> &input_data,
                                                     std::vector<unsigned char> &output_data,
                                                     const Params &params)
{
    size_t header_size = params.header.length() * 2;
    size_t footer_size = params.skip_tail ? 0 : FOOTER_SIZE;
    std::vector<unsigned char> data(input_data.begin() + header_size, input_data.end() - footer_size);
    std::vector<unsigned char> decrypted_data;
    switch (params.type)
    {
    case l2encdec::Type::XOR:
        XOR::decrypt(data, decrypted_data, params.xor_key);
        break;
    case l2encdec::Type::XOR_POSITION:
        XOR::decrypt(data, decrypted_data, params.xor_start_position, get_XOR_key_by_index);
        break;
    case l2encdec::Type::XOR_FILENAME:
        XOR::decrypt(data, decrypted_data, get_XOR_key_by_filename(params.filename));
        break;
    case l2encdec::Type::BLOWFISH:
    {
        int key_size = static_cast<int>(params.blowfish_key.length()) + BLOWFISH_KEY_NULL_TERMINATOR;
        BF::decrypt(data, decrypted_data, reinterpret_cast<const unsigned char *>(params.blowfish_key.c_str()), key_size);
        break;
    }
    case l2encdec::Type::RSA:
    {
        std::vector<unsigned char> compressed_data;
        if (RSA::decrypt(data, compressed_data, params.rsa_modulus, params.rsa_private_exponent) != 0)
            return l2encdec::DecodeResult::DECRYPTION_FAILED;
        if (ZlibUtils::unpack(compressed_data, decrypted_data) != 0)
            return l2encdec::DecodeResult::DECOMPRESSION_FAILED;
        break;
    }
    case l2encdec::Type::NONE:
        decrypted_data = input_data;
        break;
    }

    output_data = std::move(decrypted_data);

    return l2encdec::DecodeResult::SUCCESS;
}
