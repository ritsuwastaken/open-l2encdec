#include "rsa.h"
#include <cstdint>
#include <gtest/gtest.h>
#include <l2encdec.h>
#include <string>
#include <vector>

extern size_t add_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input);
extern size_t remove_padding(std::vector<uint8_t> &output, const std::vector<uint8_t> &input);

TEST(RSABasic, AddRemovePaddingRoundTrip)
{
    std::string plain = "hello RSA test";
    std::vector<uint8_t> input(plain.begin(), plain.end());
    std::vector<uint8_t> padded;
    std::vector<uint8_t> out;

    size_t padded_size = add_padding(padded, input);
    ASSERT_GT(padded_size, 0u);
    ASSERT_EQ(padded.size() % 128, 0u);

    size_t out_size = remove_padding(out, padded);
    EXPECT_EQ(out_size, input.size());
    std::string result(out.begin(), out.end());
    EXPECT_EQ(result, plain);
}

TEST(RSAEncryptDecrypt, EncryptDecryptRoundTrip)
{
    const l2encdec::Params params = {
        .rsa_modulus = "75b4d6de5c016544068a1acf125869f43d2e09fc55b8b1e289556daf9b8757635593446288b3653da1ce91c87bb1a5c18f16323495c55d7d72c0890a83f69bfd1fd9434eb1c02f3e4679edfa43309319070129c267c85604d87bb65bae205de3707af1d2108881abb567c3b3d069ae67c3a4c6a3aa93d26413d4c66094ae2039",
        .rsa_public_exponent = "30b4c2d798d47086145c75063c8e841e719776e400291d7838d3e6c4405b504c6a07f8fca27f32b86643d2649d1d5f124cdd0bf272f0909dd7352fe10a77b34d831043d9ae541f8263c6fe3d1c14c2f04e43a7253a6dda9a8c1562cbd493c1b631a1957618ad5dfe5ca28553f746e2fc6f2db816c7db223ec91e955081c1de65",
        .rsa_private_exponent = "1d",
    };
    std::vector<unsigned char> input = {'D', 'A', 'T', 'A', '1', '2', '3', '4'};
    std::vector<unsigned char> enc, dec;
    rsa::encrypt(input, enc, params.rsa_modulus, params.rsa_public_exponent);
    rsa::decrypt(enc, dec, params.rsa_modulus, params.rsa_private_exponent);
    EXPECT_EQ(dec, input);
}
