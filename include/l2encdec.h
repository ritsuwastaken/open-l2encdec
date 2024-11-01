#ifndef L2ENCDEC_PUBLIC_H
#define L2ENCDEC_PUBLIC_H

#include <iostream>
#include <vector>

#ifdef _WIN32
#ifdef L2ENCDEC_EXPORTS
#define L2ENCDEC_API __declspec(dllexport)
#else
#define L2ENCDEC_API __declspec(dllimport)
#endif
#else
#define L2ENCDEC_API
#endif

namespace l2encdec
{
    const int SUPPORTED_PROTOCOLS[] = {111, 120, 121, 211, 212, 411, 412, 413, 414};

    enum class Type
    {
        NONE,
        XOR,
        XOR_FILENAME,
        XOR_POSITION,
        BLOWFISH,
        RSA
    };

    enum class DecodeResult
    {
        SUCCESS = 0,
        INVALID_TYPE = -1,
        DECOMPRESSION_FAILED = -2,
        DECRYPTION_FAILED = -3,
        CRC32_FAILED = -4,  // Returned only when Params::skip_tail is false
        CRC32_MISMATCH = -5 // Returned only when Params::skip_tail is false
    };

    enum class EncodeResult
    {
        SUCCESS = 0,
        INVALID_TYPE = -1,
        COMPRESSION_FAILED = -2,
        CRC32_FAILED = -4 // Returned only when Params::skip_tail is false
    };

    struct Params
    {
        Type type;
        std::string header; // `decrypt`: expected header, `encrypt`: header to prepend
        bool skip_tail;     // `decrypt`: skip crc32 verification; `encrypt`: do not append tail containing crc32 checksum

        std::string filename;   // for l2encdec::Type::XOR_FILENAME
        int xor_key;            // for l2encdec::Type::XOR
        int xor_start_position; // for l2encdec::Type::XOR_POSITION

        std::string blowfish_key; // for l2encdec::Type::BLOWFISH

        std::string rsa_modulus;          // for l2encdec::Type::RSA
        std::string rsa_public_exponent;  // for l2encdec::Type::RSA, encrypt
        std::string rsa_private_exponent; // for l2encdec::Type::RSA, decrypt
    };

    /**
     * @brief Initialize default parameters for the specified protocol.
     * @param params
     * @param protocol
     * @param use_legacy_decrypt_rsa Use original private exponent and modulus for decryption, for protocols 411-414
     * @return `true` if the parameters were initialized successfully, `false` otherwise.
     */
    L2ENCDEC_API bool init_params(Params *params, int protocol, std::string filename, bool use_legacy_decrypt_rsa = false);

    /**
     * @brief Encode the input data using the specified protocol.
     */
    L2ENCDEC_API EncodeResult encode(const std::vector<unsigned char> &input_data,
                                     std::vector<unsigned char> &output_data,
                                     const Params &params);

    /**
     * @brief Decode the input data using the specified protocol.
     */
    L2ENCDEC_API DecodeResult decode(const std::vector<unsigned char> &input_data,
                                     std::vector<unsigned char> &output_data,
                                     const Params &params);
}

#endif // L2ENCDEC_PUBLIC_H
