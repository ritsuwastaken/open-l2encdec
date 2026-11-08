#include "xor_utils.h"
#include <gtest/gtest.h>
#include <vector>

struct XORCase
{
    xor_utils::Params params;
    std::vector<unsigned char> expected;
};

class XORApply : public testing::TestWithParam<XORCase>
{
};

TEST_P(XORApply, KnownBytesAndRoundTrip)
{
    const std::vector<unsigned char> input = {0, 1, 2, 3, 255};
    const auto &test = GetParam();
    std::vector<unsigned char> enc, dec;

    EXPECT_EQ(xor_utils::apply(input, enc, test.params), input.size());
    EXPECT_EQ(enc, test.expected);
    EXPECT_EQ(xor_utils::apply(enc, dec, test.params), input.size());
    EXPECT_EQ(dec, input);
}

TEST_P(XORApply, InPlace)
{
    std::vector<unsigned char> data = {0, 1, 2, 3, 255};
    EXPECT_EQ(xor_utils::apply(data, data, GetParam().params), data.size());
    EXPECT_EQ(data, GetParam().expected);
}

TEST_P(XORApply, EmptyInputClearsOutput)
{
    std::vector<unsigned char> output = {1, 2, 3};
    EXPECT_EQ(xor_utils::apply({}, output, GetParam().params), 0);
    EXPECT_TRUE(output.empty());
}

INSTANTIATE_TEST_SUITE_P(
    Modes, XORApply,
    testing::Values(
        XORCase{{.start_position = 0}, {0, 0, 0, 0, 251}},
        XORCase{{.key = 0}, {0, 1, 2, 3, 255}},
        XORCase{{.key = 0x15A}, {0x5A, 0x5B, 0x58, 0x59, 0xA5}},
        XORCase{{.filename = "test.txt"}, {0x4E, 0x4F, 0x4C, 0x4D, 0xB1}},
        XORCase{{.filename = "TeSt.TXT"}, {0x4E, 0x4F, 0x4C, 0x4D, 0xB1}},
        XORCase{{.start_position = 0xFE}, {0xFE, 0xFE, 0x03, 0x03, 0xFC}}));
