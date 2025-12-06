#include "xor_utils.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(XORBasic, KeyByIndexZero)
{
    EXPECT_EQ(xor_utils::get_key_by_index(0), 0);
}

TEST(XORBasic, KeyByFilenameCaseInsensitive)
{
    int k1 = xor_utils::get_key_by_filename(std::string("TeSt.TXT"));
    int k2 = xor_utils::get_key_by_filename(std::string("test.txt"));
    EXPECT_EQ(k1, k2);
}

TEST(XOREncryptDecrypt, SymmetricSingleKey)
{
    std::vector<unsigned char> input = {'h', 'e', 'l', 'l', 'o'};
    std::vector<unsigned char> enc, dec;
    int key = 0x5A;
    xor_utils::encrypt(input, enc, key);
    ASSERT_EQ(enc.size(), input.size());
    xor_utils::decrypt(enc, dec, key);
    EXPECT_EQ(dec, input);
}

TEST(XOREncryptDecrypt, SymmetricKeyGenerator)
{
    std::vector<unsigned char> input = {1, 2, 3, 4, 5};
    std::vector<unsigned char> enc, dec;
    xor_utils::KeyGenerator kg = [](int idx)
    { return idx & 0xFF; };
    xor_utils::encrypt(input, enc, 10, kg);
    ASSERT_EQ(enc.size(), input.size());
    xor_utils::decrypt(enc, dec, 10, kg);
    EXPECT_EQ(dec, input);
}
