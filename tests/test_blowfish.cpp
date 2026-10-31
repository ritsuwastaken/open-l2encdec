#include "blowfish.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(BFEncryptDecrypt, EncryptDecryptRoundTrip)
{
    std::string key = "testkey";
    std::vector<unsigned char> input = {'D', 'A', 'T', 'A', '1', '2', '3', '4'};
    std::vector<unsigned char> enc, dec;

    size_t enc_size = blowfish::encrypt(input, enc, key);
    ASSERT_EQ(enc_size, enc.size());
    ASSERT_EQ(enc.size(), input.size());

    size_t dec_size = blowfish::decrypt(enc, dec, key);
    ASSERT_EQ(dec_size, dec.size());
    EXPECT_EQ(dec, input);
}

TEST(BFEncryptDecrypt, RejectsOversizedKey)
{
    std::vector<unsigned char> input(8), output = {1, 2, 3};
    EXPECT_EQ(blowfish::encrypt(input, output, std::string(56, 'a')), size_t(-1));
    EXPECT_TRUE(output.empty());
    EXPECT_EQ(blowfish::decrypt(input, output, std::string(56, 'a')), size_t(-1));
    EXPECT_TRUE(output.empty());
}
