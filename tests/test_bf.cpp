#include "bf.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(BFEncryptDecrypt, EncryptDecryptRoundTrip)
{
    std::string key = "testkey";
    std::vector<unsigned char> input = {'D', 'A', 'T', 'A', '1', '2', '3', '4'};
    std::vector<unsigned char> enc, dec;

    size_t enc_size = BF::encrypt(input, enc, key);
    ASSERT_EQ(enc_size, enc.size());
    ASSERT_EQ(enc.size(), input.size());

    size_t dec_size = BF::decrypt(enc, dec, key);
    ASSERT_EQ(dec_size, dec.size());
    EXPECT_EQ(dec, input);
}
