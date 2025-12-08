#include <gtest/gtest.h>
#include <l2encdec.h>

static std::vector<unsigned char> make_input()
{
    return {'T', 'E', 'S', 'T', '1', '2', '3'};
}

TEST(L2EncodeDecode, XOR)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;

    ASSERT_EQ(l2encdec::encode(input, enc, 111, "", false),
              l2encdec::EncodeResult::SUCCESS);

    ASSERT_EQ(l2encdec::decode(enc, dec, 111, "", false),
              l2encdec::DecodeResult::SUCCESS);

    EXPECT_EQ(dec, input);
}

TEST(L2EncodeDecode, XORPosition)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;

    ASSERT_EQ(l2encdec::encode(input, enc, 120, "", false),
              l2encdec::EncodeResult::SUCCESS);

    ASSERT_EQ(l2encdec::decode(enc, dec, 120, "", false),
              l2encdec::DecodeResult::SUCCESS);

    EXPECT_EQ(dec, input);
}

TEST(L2EncodeDecode, XORFilename)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;

    ASSERT_EQ(l2encdec::encode(input, enc, 121, "file.txt", false),
              l2encdec::EncodeResult::SUCCESS);

    ASSERT_EQ(l2encdec::decode(enc, dec, 121, "file.txt", false),
              l2encdec::DecodeResult::SUCCESS);

    EXPECT_EQ(dec, input);
}

TEST(L2EncodeDecode, Blowfish)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;

    ASSERT_EQ(l2encdec::encode(input, enc, 211, "", false),
              l2encdec::EncodeResult::SUCCESS);

    ASSERT_EQ(l2encdec::decode(enc, dec, 211, "", false),
              l2encdec::DecodeResult::SUCCESS);

    EXPECT_EQ(dec, input);
}

TEST(L2EncodeDecode, RSA)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;

    ASSERT_EQ(l2encdec::encode(input, enc, 411, "", false),
              l2encdec::EncodeResult::SUCCESS);

    ASSERT_EQ(l2encdec::decode(enc, dec, 411, "", false),
              l2encdec::DecodeResult::SUCCESS);

    EXPECT_EQ(dec, input);
}

TEST(L2Encode, HeaderAndTailApplied)
{
    auto input = make_input();
    std::vector<unsigned char> enc;

    ASSERT_EQ(l2encdec::encode(input, enc, 111, "", false),
              l2encdec::EncodeResult::SUCCESS);

    int footer_size = 20;
    const std::string header = "Lineage2Ver111";
    ASSERT_GE(enc.size(), header.size() * 2 + footer_size);

    for (size_t i = 0; i < header.size(); ++i)
    {
        EXPECT_EQ(enc[i * 2], static_cast<unsigned char>(header[i]));
        EXPECT_EQ(enc[i * 2 + 1], 0);
    }
}

TEST(L2DecodeFail, TooSmallInput)
{
    std::vector<unsigned char> input = {1, 2};
    std::vector<unsigned char> out;

    EXPECT_EQ(l2encdec::decode(input, out, 111, "", false),
              l2encdec::DecodeResult::INVALID_TYPE);
}
