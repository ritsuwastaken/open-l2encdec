#include "l2encdec.h"
#include <emscripten/bind.h>

using namespace emscripten;

namespace
{
[[noreturn]] void throwJsError(const char *name, int code, const std::string &message)
{
    val err = val::global("Error").new_(message);
    err.set("name", name);
    err.set("code", code);
    err.throw_();
}

void throwEncodeError(l2encdec::EncodeResult r)
{
    switch (r)
    {
    case l2encdec::EncodeResult::INVALID_TYPE:
        throwJsError("EncodeError", static_cast<int>(r), "Invalid encryption type");
    case l2encdec::EncodeResult::COMPRESSION_FAILED:
        throwJsError("EncodeError", static_cast<int>(r), "Compression failed");
    case l2encdec::EncodeResult::ENCRYPTION_FAILED:
        throwJsError("EncodeError", static_cast<int>(r), "Encryption failed");
    default:
        throwJsError("EncodeError", static_cast<int>(r), "Unknown encode error");
    }
}

void throwDecodeError(l2encdec::DecodeResult r)
{
    switch (r)
    {
    case l2encdec::DecodeResult::INVALID_TYPE:
        throwJsError("DecodeError", static_cast<int>(r), "Invalid encryption type");
    case l2encdec::DecodeResult::DECOMPRESSION_FAILED:
        throwJsError("DecodeError", static_cast<int>(r), "Decompression failed");
    case l2encdec::DecodeResult::DECRYPTION_FAILED:
        throwJsError("DecodeError", static_cast<int>(r), "Decryption failed");
    default:
        throwJsError("DecodeError", static_cast<int>(r), "Unknown decode error");
    }
}

void throwChecksumError(l2encdec::ChecksumResult r)
{
    if (r == l2encdec::ChecksumResult::MISMATCH)
        throwJsError("ChecksumError", static_cast<int>(r), "Checksum mismatch");
}

} // namespace

EMSCRIPTEN_BINDINGS(l2encdec_bindings)
{
    enum_<l2encdec::Type>("Type")
        .value("NONE", l2encdec::Type::NONE)
        .value("XOR", l2encdec::Type::XOR)
        .value("XOR_FILENAME", l2encdec::Type::XOR_FILENAME)
        .value("XOR_POSITION", l2encdec::Type::XOR_POSITION)
        .value("BLOWFISH", l2encdec::Type::BLOWFISH)
        .value("RSA", l2encdec::Type::RSA);

    enum_<l2encdec::EncodeResult>("EncodeResult")
        .value("SUCCESS", l2encdec::EncodeResult::SUCCESS)
        .value("INVALID_TYPE", l2encdec::EncodeResult::INVALID_TYPE)
        .value("COMPRESSION_FAILED", l2encdec::EncodeResult::COMPRESSION_FAILED)
        .value("ENCRYPTION_FAILED", l2encdec::EncodeResult::ENCRYPTION_FAILED);

    enum_<l2encdec::DecodeResult>("DecodeResult")
        .value("SUCCESS", l2encdec::DecodeResult::SUCCESS)
        .value("INVALID_TYPE", l2encdec::DecodeResult::INVALID_TYPE)
        .value("DECOMPRESSION_FAILED", l2encdec::DecodeResult::DECOMPRESSION_FAILED)
        .value("DECRYPTION_FAILED", l2encdec::DecodeResult::DECRYPTION_FAILED);

    enum_<l2encdec::ChecksumResult>("ChecksumResult")
        .value("SUCCESS", l2encdec::ChecksumResult::SUCCESS)
        .value("MISMATCH", l2encdec::ChecksumResult::MISMATCH);

    class_<l2encdec::Params>("Params")
        .constructor<>()
        .property("type", &l2encdec::Params::type)
        .property("protocol", &l2encdec::Params::protocol)
        .property("header", &l2encdec::Params::header)
        .property("tail", &l2encdec::Params::tail)
        .property("skip_tail", &l2encdec::Params::skip_tail)
        .property("skip_header", &l2encdec::Params::skip_header)
        .property("filename", &l2encdec::Params::filename)
        .property("xor_key", &l2encdec::Params::xor_key)
        .property("xor_start_position", &l2encdec::Params::xor_start_position)
        .property("blowfish_key", &l2encdec::Params::blowfish_key)
        .property("rsa_modulus", &l2encdec::Params::rsa_modulus)
        .property("rsa_public_exponent", &l2encdec::Params::rsa_public_exponent)
        .property("rsa_private_exponent", &l2encdec::Params::rsa_private_exponent);

    function(
        "init_params",
        optional_override([](l2encdec::Params &p, int protocol, const std::string &filename, bool legacy)
                          {
            if (!l2encdec::init_params(p, protocol, filename, legacy))
                throwJsError("InitParamsError", -1, "Failed to initialize parameters"); }));

    function(
        "verify_checksum",
        optional_override([](val inputJs)
                          {
            auto input = vecFromJSArray<unsigned char>(inputJs);
            if (auto r = l2encdec::verify_checksum(input); r != l2encdec::ChecksumResult::SUCCESS)
                throwChecksumError(r); }));

    function(
        "encode",
        optional_override([](val inputJs, const l2encdec::Params &p)
                          {
            auto input = vecFromJSArray<unsigned char>(inputJs);
            std::vector<unsigned char> out;

            if (auto r = l2encdec::encode(input, out, p); r != l2encdec::EncodeResult::SUCCESS)
                throwEncodeError(r);

            return val(typed_memory_view(out.size(), out.data())); }));

    function(
        "decode",
        optional_override([](val inputJs, const l2encdec::Params &p)
                          {
            auto input = vecFromJSArray<unsigned char>(inputJs);
            std::vector<unsigned char> out;

            if (auto r = l2encdec::decode(input, out, p); r != l2encdec::DecodeResult::SUCCESS)
                throwDecodeError(r);

            return val(typed_memory_view(out.size(), out.data())); }));
}
