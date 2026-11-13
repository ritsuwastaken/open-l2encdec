#include "l2encdec.h"
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_DECLARE_VAL_TYPE(ByteArray);

namespace
{
ByteArray copyBytes(const std::vector<unsigned char> &bytes)
{
    return ByteArray(val::global("Uint8Array").new_(val(typed_memory_view(bytes.size(), bytes.data()))));
}

val result(int code)
{
    val r = val::object();
    r.set("code", code);
    return r;
}

val result(int code, const std::vector<unsigned char> &bytes)
{
    val r = result(code);
    if (code == 0)
        r.set("data", copyBytes(bytes));
    return r;
}
} // namespace

EMSCRIPTEN_BINDINGS(l2encdec_bindings)
{
    register_type<ByteArray>("Uint8Array");
    enum_<l2encdec::Type>("Type", enum_value_type::number)
        .value("NONE", l2encdec::Type::NONE)
        .value("XOR", l2encdec::Type::XOR)
        .value("XOR_FILENAME", l2encdec::Type::XOR_FILENAME)
        .value("XOR_POSITION", l2encdec::Type::XOR_POSITION)
        .value("BLOWFISH", l2encdec::Type::BLOWFISH)
        .value("RSA", l2encdec::Type::RSA);

    enum_<l2encdec::EncodeResult>("EncodeResult", enum_value_type::number)
        .value("SUCCESS", l2encdec::EncodeResult::SUCCESS)
        .value("INVALID_TYPE", l2encdec::EncodeResult::INVALID_TYPE)
        .value("COMPRESSION_FAILED", l2encdec::EncodeResult::COMPRESSION_FAILED)
        .value("ENCRYPTION_FAILED", l2encdec::EncodeResult::ENCRYPTION_FAILED)
        .value("CANCELLED", l2encdec::EncodeResult::CANCELLED);

    enum_<l2encdec::DecodeResult>("DecodeResult", enum_value_type::number)
        .value("SUCCESS", l2encdec::DecodeResult::SUCCESS)
        .value("INVALID_TYPE", l2encdec::DecodeResult::INVALID_TYPE)
        .value("DECOMPRESSION_FAILED", l2encdec::DecodeResult::DECOMPRESSION_FAILED)
        .value("DECRYPTION_FAILED", l2encdec::DecodeResult::DECRYPTION_FAILED)
        .value("CANCELLED", l2encdec::DecodeResult::CANCELLED);

    enum_<l2encdec::ChecksumResult>("ChecksumResult", enum_value_type::number)
        .value("SUCCESS", l2encdec::ChecksumResult::SUCCESS)
        .value("MISMATCH", l2encdec::ChecksumResult::MISMATCH);

    value_object<l2encdec::Params>("Params")
        .field("type", &l2encdec::Params::type)
        .field("protocol", &l2encdec::Params::protocol)
        .field("header", &l2encdec::Params::header)
        .field("tail", &l2encdec::Params::tail)
        .field("skip_tail", &l2encdec::Params::skip_tail)
        .field("skip_header", &l2encdec::Params::skip_header)
        .field("filename", &l2encdec::Params::filename)
        .field("xor_key", &l2encdec::Params::xor_key)
        .field("xor_start_position", &l2encdec::Params::xor_start_position)
        .field("blowfish_key", &l2encdec::Params::blowfish_key)
        .field("rsa_modulus", &l2encdec::Params::rsa_modulus)
        .field("rsa_public_exponent", &l2encdec::Params::rsa_public_exponent)
        .field("rsa_private_exponent", &l2encdec::Params::rsa_private_exponent);

    function(
        "init_params",
        optional_override([](int protocol, const std::string &filename, bool legacy)
                          {
            l2encdec::Params p{};
            if (!l2encdec::init_params(p, protocol, filename, legacy))
                return val::null();
            return val(p); }));

    function(
        "verify_checksum",
        optional_override([](val inputJs)
                          {
            auto input = convertJSArrayToNumberVector<unsigned char>(inputJs);
            return static_cast<int>(l2encdec::verify_checksum(input)); }));

    function(
        "encode",
        optional_override([](val inputJs, const l2encdec::Params &p)
                          {
            auto input = convertJSArrayToNumberVector<unsigned char>(inputJs);
            std::vector<unsigned char> out;
            const auto r = l2encdec::encode(input, out, p);
            return result(static_cast<int>(r), out); }));

    function(
        "decode",
        optional_override([](val inputJs, const l2encdec::Params &p)
                          {
            auto input = convertJSArrayToNumberVector<unsigned char>(inputJs);
            std::vector<unsigned char> out;
            const auto r = l2encdec::decode(input, out, p);
            return result(static_cast<int>(r), out); }));
}
