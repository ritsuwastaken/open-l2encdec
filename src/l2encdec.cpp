#include "blowfish.h"
#include "l2encdec_private.h" // IWYU pragma: keep
#include "rsa.h"
#include "xor_utils.h"
#include "zlib_utils.h"
#include <algorithm>
#include <cstddef>
#include <cstring>

constexpr size_t FOOTER_SIZE = 20;
constexpr size_t FOOTER_CRC32_OFFSET = 12;
constexpr std::string_view HEADER_PREFIX = "Lineage2Ver";
constexpr size_t PROTOCOL_SIZE = 3;
constexpr size_t HEADER_SIZE = (HEADER_PREFIX.size() + PROTOCOL_SIZE) * 2;

const std::unordered_map<int, l2encdec::Params> PROTOCOL_CONFIGS = {
    {111, {.type = l2encdec::Type::XOR, .xor_key = 0xAC}},
    {120, {.type = l2encdec::Type::XOR_POSITION, .xor_start_position = 0xE6}},
    {121, {.type = l2encdec::Type::XOR_FILENAME}},
    {211, {.type = l2encdec::Type::BLOWFISH, .blowfish_key = "31==-%&@!^+][;'.]94-"}},
    {212, {.type = l2encdec::Type::BLOWFISH, .blowfish_key = "[;'.]94-&@%!^+]-31=="}},
    {411, {.type = l2encdec::Type::RSA, .rsa_modulus = "8c9d5da87b30f5d7cd9dc88c746eaac5bb180267fa11737358c4c95d9adf59dd37689f9befb251508759555d6fe0eca87bebe0a10712cf0ec245af84cd22eb4cb675e98eaf5799fca62a20a2baa4801d5d70718dcd43283b8428f1387aec6600f937bfc7bb72404d187d3a9c438f1ffce9ce365dccf754232ff6def038a41385", .rsa_private_exponent = "1d"}},
    {412, {.type = l2encdec::Type::RSA, .rsa_modulus = "a465134799cf2c45087093e7d0f0f144e6d528110c08f674730d436e40827330eccea46e70acf10cdda7d8f710e3b44dcca931812d76cd7494289bca8b73823f57efc0515b97e4a2a02612ccfa719cf7885104b06f2e7e2cc967b62e3d3b1aadb925db94cbc8cd3070a4bb13f7e202c7733a67b1b94c1ebc0afcbe1a63b448cf", .rsa_private_exponent = "25"}},
    {413, {.type = l2encdec::Type::RSA, .rsa_modulus = "97df398472ddf737ef0a0cd17e8d172f0fef1661a38a8ae1d6e829bc1c6e4c3cfc19292dda9ef90175e46e7394a18850b6417d03be6eea274d3ed1dde5b5d7bde72cc0a0b71d03608655633881793a02c9a67d9ef2b45eb7c08d4be329083ce450e68f7867b6749314d40511d09bc5744551baa86a89dc38123dc1668fd72d83", .rsa_private_exponent = "35"}},
    {414, {.type = l2encdec::Type::RSA, .rsa_modulus = "ad70257b2316ce09dfaf2ebc3f63b3d673b0c98a403950e26bb87379b11e17aed0e45af23e7171e5ec1fbc8d1ae32ffb7801b31266eef9c334b53469d4b7cbe83284273d35a9aab49b453e7012f374496c65f8089f5d134b0eb3d1e3b22051ed5977a6dd68c4f85785dfcc9f4412c81681944fc4b8ce27caf0242deaa5762e8d", .rsa_private_exponent = "25"}}};

const l2encdec::Params MODERN_RSA_PARAMS = {
    .type = l2encdec::Type::RSA,
    .rsa_modulus = "75b4d6de5c016544068a1acf125869f43d2e09fc55b8b1e289556daf9b8757635593446288b3653da1ce91c87bb1a5c18f16323495c55d7d72c0890a83f69bfd1fd9434eb1c02f3e4679edfa43309319070129c267c85604d87bb65bae205de3707af1d2108881abb567c3b3d069ae67c3a4c6a3aa93d26413d4c66094ae2039",
    .rsa_public_exponent = "30b4c2d798d47086145c75063c8e841e719776e400291d7838d3e6c4405b504c6a07f8fca27f32b86643d2649d1d5f124cdd0bf272f0909dd7352fe10a77b34d831043d9ae541f8263c6fe3d1c14c2f04e43a7253a6dda9a8c1562cbd493c1b631a1957618ad5dfe5ca28553f746e2fc6f2db816c7db223ec91e955081c1de65",
    .rsa_private_exponent = "1d",
};

inline void insert_header(std::vector<unsigned char> &data, std::string_view header)
{
    const size_t wide_size = header.size() * 2;
    data.reserve(data.size() + wide_size);

    std::vector<unsigned char> wide;
    wide.resize(wide_size);

    for (size_t i = 0; i < header.size(); i++)
    {
        wide[i * 2] = static_cast<unsigned char>(header[i]);
        wide[i * 2 + 1] = 0;
    }

    data.insert(data.begin(), wide.begin(), wide.end());
}

inline void insert_tail(std::vector<unsigned char> &data, uint32_t crc)
{
    const size_t start = data.size();
    data.resize(start + FOOTER_SIZE);
    unsigned char *footer = data.data() + start;
    uint32_t *crc_ptr = reinterpret_cast<uint32_t *>(footer + FOOTER_CRC32_OFFSET);
    std::memcpy(crc_ptr, &crc, sizeof(crc));
}

inline void insert_tail(std::vector<unsigned char> &data, std::string_view tail)
{
    std::string padded = std::string(tail);
    if (padded.size() % 2 != 0)
        padded.insert(padded.begin(), '0');

    const size_t byte_count = padded.size() / 2;
    data.reserve(data.size() + byte_count);

    for (size_t i = 0; i < padded.size(); i += 2)
    {
        unsigned char value = static_cast<unsigned char>(std::stoi(padded.substr(i, 2), nullptr, 16));
        data.push_back(value);
    }
}

// TODO: remove
inline int get_protocol_from_params(const l2encdec::Params &p)
{
    if (!p.header.empty() &&
        p.header.size() >= HEADER_PREFIX.size() + PROTOCOL_SIZE)
    {
        std::string digits =
            p.header.substr(p.header.size() - PROTOCOL_SIZE, PROTOCOL_SIZE);

        if (std::all_of(digits.begin(), digits.end(), ::isdigit))
            return std::stoi(digits);
    }

    for (const auto &kv : PROTOCOL_CONFIGS)
    {
        int proto = kv.first;
        const l2encdec::Params &cfg = kv.second;

        if (p.type != cfg.type)
            continue;

        switch (p.type)
        {
        case l2encdec::Type::XOR:
            if (p.xor_key == cfg.xor_key)
                return proto;
            break;

        case l2encdec::Type::XOR_POSITION:
            if (p.xor_start_position == cfg.xor_start_position)
                return proto;
            break;

        case l2encdec::Type::XOR_FILENAME:
            return proto;

        case l2encdec::Type::BLOWFISH:
            if (p.blowfish_key == cfg.blowfish_key)
                return proto;
            break;

        case l2encdec::Type::RSA:
            if (p.rsa_modulus == cfg.rsa_modulus)
                return proto;
            break;

        default:
            break;
        }
    }

    return -1;
}

L2ENCDEC_API bool l2encdec::init_params(Params *params, int protocol, const std::string &filename, bool use_legacy_rsa)
{
    if (protocol == 121 && filename.empty())
        return false;

    auto it = PROTOCOL_CONFIGS.find(protocol);
    if (it == PROTOCOL_CONFIGS.end())
        return false;

    *params = it->second;

    if (params->type == Type::RSA && !use_legacy_rsa)
        *params = MODERN_RSA_PARAMS;

    params->filename = filename;
    params->header = std::string(HEADER_PREFIX) + std::to_string(protocol);

    return true;
}

L2ENCDEC_API l2encdec::ChecksumResult l2encdec::verify_checksum(const std::vector<unsigned char> &input)
{
    uint32_t checksum = *reinterpret_cast<const uint32_t *>(input.data() + input.size() - FOOTER_SIZE + FOOTER_CRC32_OFFSET);
    std::vector<unsigned char> nofooter(input.begin(), input.end() - FOOTER_SIZE);
    uint32_t calc = zlib_utils::checksum(nofooter);
    return calc == checksum ? ChecksumResult::SUCCESS : ChecksumResult::MISMATCH;
}

L2ENCDEC_API l2encdec::EncodeResult l2encdec::encode(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, const Params &p)
{
    std::vector<unsigned char> enc;
    switch (p.type)
    {
    case Type::XOR:
        xor_utils::apply(input, enc, p.xor_key);
        break;
    case Type::XOR_FILENAME:
        xor_utils::apply(input, enc, xor_utils::get_key_by_filename(p.filename));
        break;
    case Type::XOR_POSITION:
        xor_utils::apply(input, enc, p.xor_start_position, xor_utils::get_key_by_index);
        break;
    case Type::BLOWFISH:
        blowfish::encrypt(input, enc, p.blowfish_key);
        break;
    case Type::RSA:
    {
        std::vector<unsigned char> compressed;
        if (zlib_utils::pack(input, compressed) != 0)
            return EncodeResult::COMPRESSION_FAILED;
        rsa::encrypt(compressed, enc, p.rsa_modulus, p.rsa_public_exponent);
        break;
    }
    default:
        enc = input;
        break;
    }

    if (!p.skip_header)
    {
        if (p.header.empty())
        {
            int protocol = get_protocol_from_params(p);
            if (protocol == -1)
                return EncodeResult::INVALID_TYPE;
            std::string protocol_string = std::string(HEADER_PREFIX) + std::to_string(protocol);
            insert_header(enc, protocol_string);
        }
        else
            insert_header(enc, p.header);
    }

    if (!p.skip_tail)
    {
        if (p.tail.empty())
            insert_tail(enc, zlib_utils::checksum(enc));
        else
            insert_tail(enc, p.tail);
    }

    output = std::move(enc);
    return EncodeResult::SUCCESS;
}

L2ENCDEC_API l2encdec::DecodeResult l2encdec::decode(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, const Params &p)
{
    size_t header_size = p.skip_header ? 0 : !p.header.empty() ? p.header.size() * 2
                                                               : HEADER_SIZE;
    size_t footer_size = p.skip_tail ? 0 : !p.tail.empty() ? p.tail.size() / 2
                                                           : FOOTER_SIZE;
    if (input.size() < header_size + footer_size)
        return DecodeResult::DECRYPTION_FAILED;

    std::vector<unsigned char> data(input.begin() + header_size, input.end() - footer_size);
    std::vector<unsigned char> dec;
    switch (p.type)
    {
    case Type::XOR:
        xor_utils::apply(data, dec, p.xor_key);
        break;
    case Type::XOR_POSITION:
        xor_utils::apply(data, dec, p.xor_start_position, xor_utils::get_key_by_index);
        break;
    case Type::XOR_FILENAME:
        xor_utils::apply(data, dec, xor_utils::get_key_by_filename(p.filename));
        break;
    case Type::BLOWFISH:
        blowfish::decrypt(data, dec, p.blowfish_key);
        break;
    case Type::RSA:
    {
        std::vector<unsigned char> compressed;
        if (rsa::decrypt(data, compressed, p.rsa_modulus, p.rsa_private_exponent) != 0)
            return DecodeResult::DECRYPTION_FAILED;
        if (zlib_utils::unpack(compressed, dec) != 0)
            return DecodeResult::DECOMPRESSION_FAILED;
        break;
    }
    default:
        dec = input;
        break;
    }

    output = std::move(dec);
    return DecodeResult::SUCCESS;
}

L2ENCDEC_API l2encdec::EncodeResult l2encdec::encode_auto(
    const std::vector<unsigned char> &input,
    std::vector<unsigned char> &output,
    int protocol,
    const std::string &filename,
    bool use_legacy_rsa)
{
    l2encdec::Params p{};
    if (!l2encdec::init_params(&p, protocol, filename, use_legacy_rsa))
        return EncodeResult::INVALID_TYPE;

    return l2encdec::encode(input, output, p);
}

L2ENCDEC_API l2encdec::DecodeResult l2encdec::decode_auto(
    const std::vector<unsigned char> &input,
    std::vector<unsigned char> &output,
    int protocol,
    const std::string &filename,
    bool use_legacy_rsa)
{
    if (input.size() >= HEADER_SIZE && protocol == -1)
    {
        std::string ascii;
        ascii.reserve(HEADER_PREFIX.size() + PROTOCOL_SIZE);

        for (size_t i = 0; i < HEADER_SIZE; i += 2)
            ascii.push_back(static_cast<char>(input[i]));

        if (ascii.compare(0, HEADER_PREFIX.size(), HEADER_PREFIX) == 0)
        {
            std::string digits = ascii.substr(HEADER_PREFIX.size(), PROTOCOL_SIZE);
            if (std::all_of(digits.begin(), digits.end(), ::isdigit))
                protocol = std::stoi(digits);
        }
    }

    l2encdec::Params p{};
    if (protocol > 0)
    {
        if (!l2encdec::init_params(&p, protocol, filename, use_legacy_rsa))
            return DecodeResult::INVALID_TYPE;
    }
    else
    {
        bool found = false;
        for (auto &kv : PROTOCOL_CONFIGS)
        {
            if (init_params(&p, kv.first, filename, use_legacy_rsa))
            {
                protocol = kv.first;
                found = true;
                break;
            }
        }
        if (!found)
            return DecodeResult::INVALID_TYPE;
    }

    return decode(input, output, p);
}
