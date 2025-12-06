#ifndef L2ENCDEC_PUBLIC_H
#define L2ENCDEC_PUBLIC_H

#ifndef L2ENCDEC_API
#define L2ENCDEC_API
#endif

#include <string>
#include <vector>

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

enum class ChecksumResult
{
    SUCCESS = 0,
    MISMATCH = -1
};

enum class DecodeResult
{
    SUCCESS = 0,
    INVALID_TYPE = -1,
    DECOMPRESSION_FAILED = -2,
    DECRYPTION_FAILED = -3,
};

enum class EncodeResult
{
    SUCCESS = 0,
    INVALID_TYPE = -1,
    COMPRESSION_FAILED = -2,
};

struct Params
{
    Type type;
    std::string header;               // `decrypt`: expected header, `encrypt`: header to prepend
    std::string tail;                 // `encode`: custom tail
    bool skip_tail = false;           // `decrypt`: read file without tail; `encrypt`: do not append tail
    bool skip_header = false;         // `decrypt`: ignore/prevent header handling; `encrypt`: do not prepend header
    std::string filename;             // for l2encdec::Type::XOR_FILENAME
    int xor_key;                      // for l2encdec::Type::XOR
    int xor_start_position;           // for l2encdec::Type::XOR_POSITION
    std::string blowfish_key;         // for l2encdec::Type::BLOWFISH
    std::string rsa_modulus;          // for l2encdec::Type::RSA
    std::string rsa_public_exponent;  // for l2encdec::Type::RSA, encrypt
    std::string rsa_private_exponent; // for l2encdec::Type::RSA, decrypt
};

/**
 * @brief Initialize default parameters for the specified protocol.
 * @param params Struct to populate with parameters
 * @param protocol Last three digits of the file header
 * @param filename Filename used to calculate the XOR key for protocol 121
 * @param use_legacy_decrypt_rsa Use original private exponent and modulus for decryption, for protocols 411-414
 * @return `true` if the parameters were initialized successfully, `false` otherwise.
 */
L2ENCDEC_API bool init_params(Params *params, int protocol, const std::string &filename = "", bool use_legacy_decrypt_rsa = false);

/**
 * @brief Verify the checksum of the input data.
 */
L2ENCDEC_API ChecksumResult verify_checksum(const std::vector<unsigned char> &input_data);

/**
 * @brief Encode the input data using params.
 */
L2ENCDEC_API EncodeResult encode(const std::vector<unsigned char> &input_data,
                                 std::vector<unsigned char> &output_data,
                                 const Params &params);

/**
 * @brief Decode the input data using params.
 */
L2ENCDEC_API DecodeResult decode(const std::vector<unsigned char> &input_data,
                                 std::vector<unsigned char> &output_data,
                                 const Params &params);

/**
 * @brief Decode input data using automatic protocol detection.
 *
 * This function attempts to determine the correct Lineage 2 protocol
 * by inspecting the encrypted file header (unless @p skip_header is true).
 * If a protocol is explicitly provided via @p protocol, it is used instead.
 *
 * After determining the protocol, default parameters are initialized via
 * init_params(), and decode() is performed using those parameters.
 *
 * @param input        Raw encoded/encrypted file contents.
 * @param output       Buffer receiving the decoded/processed data.
 * @param filename     Used only for protocol 121 (XOR_FILENAME), ignored otherwise.
 * @param protocol     Optional override; if >0, this protocol is used directly.
 *                    If -1, protocol is extracted from the header or inferred.
 * @param skip_header  If true, header bytes are not read or stripped during decode.
 * @param skip_tail    If true, the footer checksum/tail is not removed.
 * @param use_legacy_rsa  If true, use legacy RSA parameters for 411–414.
 *
 * @return DecodeResult::SUCCESS on success, or an error code on failure.
 */
L2ENCDEC_API DecodeResult decode_auto(
    const std::vector<unsigned char> &input,
    std::vector<unsigned char> &output,
    const std::string &filename, // only needed for protocol 121
    int protocol = -1,
    bool skip_header = false,
    bool skip_tail = false,
    bool use_legacy_rsa = false);

/**
 * @brief Encode input data using automatic parameter initialization.
 *
 * This function initializes a parameter set for the specified protocol using
 * init_params(), applies optional header/tail suppression, and performs encode().
 *
 * Protocol must be explicitly provided for encoding, unlike decode_auto(),
 * because the intended output format cannot be inferred from the input.
 *
 * @param input        Raw data to encode/encrypt.
 * @param output       Buffer receiving the encoded/encrypted data.
 * @param filename     Used only for protocol 121 (XOR_FILENAME), ignored otherwise.
 * @param protocol     Protocol number (e.g. 111, 211, 413). Must be valid.
 * @param skip_header  If true, the header prefix is not prepended.
 * @param skip_tail    If true, the footer checksum/tail is not appended.
 * @param use_legacy_rsa  If true, use legacy RSA parameters for 411–414.
 *
 * @return EncodeResult::SUCCESS on success, or an error code on failure.
 */
L2ENCDEC_API EncodeResult encode_auto(
    const std::vector<unsigned char> &input,
    std::vector<unsigned char> &output,
    const std::string &filename, // only used for protocol 121
    int protocol,
    bool skip_header,
    bool skip_tail,
    bool use_legacy_rsa);
} // namespace l2encdec

#endif // L2ENCDEC_PUBLIC_H
