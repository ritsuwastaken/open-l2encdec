#include <emscripten/bind.h>
#include "l2encdec.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(l2encdec_bindings) {
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

    function("init_params", &l2encdec::init_params);

    function("encode", optional_override([](val inputJs, l2encdec::Params p) {
        std::vector<unsigned char> input = vecFromJSArray<unsigned char>(inputJs);
        std::vector<unsigned char> out;
        l2encdec::encode(input, out, p);
        return val::array(out);
    }));

    function("decode", optional_override([](val inputJs, l2encdec::Params p) {
        std::vector<unsigned char> input = vecFromJSArray<unsigned char>(inputJs);
        std::vector<unsigned char> out;
        l2encdec::decode(input, out, p);
        return val::array(out);
    }));
}
