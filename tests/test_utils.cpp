#include "utils.h"
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(UtilsHeader, AddHeaderBasic)
{
    std::vector<unsigned char> data = {0xAA, 0xBB};
    utils::add_header(data, "AB");

    std::vector<unsigned char> expected = {
        0x41, 0x00,
        0x42, 0x00,
        0xAA, 0xBB};

    EXPECT_EQ(data, expected);
}

TEST(UtilsHeader, AddHeaderEmpty)
{
    std::vector<unsigned char> data = {0x11, 0x22};
    utils::add_header(data, "");

    std::vector<unsigned char> expected = {0x11, 0x22};
    EXPECT_EQ(data, expected);
}

TEST(UtilsTail, MakeTailBasic)
{
    constexpr uint32_t crc = 0x12345678;
    constexpr size_t offset = 2;
    constexpr size_t size = 8;

    std::string tail = utils::make_tail(crc, offset, size);

    ASSERT_EQ(tail.size(), 16u);

    std::vector<unsigned char> raw(size, 0);
    std::memcpy(raw.data() + offset, &crc, sizeof(crc));

    std::string expected;
    expected.reserve(size * 2);
    for (unsigned char b : raw)
        expected += std::format("{:02X}", b);

    EXPECT_EQ(tail, expected);
}

TEST(UtilsTail, AddTailBasic)
{
    std::vector<unsigned char> data = {0xAA, 0xBB};
    utils::add_tail(data, "0102FF");

    std::vector<unsigned char> expected = {
        0xAA, 0xBB,
        0x01, 0x02, 0xFF};

    EXPECT_EQ(data, expected);
}

TEST(UtilsTail, AddTailOddLengthPadding)
{
    std::vector<unsigned char> data;
    utils::add_tail(data, "A"); // padded to "0A"

    std::vector<unsigned char> expected = {0x0A};
    EXPECT_EQ(data, expected);
}

TEST(UtilsTail, MakeTailAddTailRoundTrip)
{
    constexpr uint32_t crc = 0xDEADBEEF;
    constexpr size_t offset = 4;
    constexpr size_t size = 16;

    std::vector<unsigned char> data;

    std::string tail = utils::make_tail(crc, offset, size);
    utils::add_tail(data, tail);

    ASSERT_EQ(data.size(), size);

    uint32_t extracted = 0;
    std::memcpy(&extracted, data.data() + offset, sizeof(extracted));

    EXPECT_EQ(extracted, crc);
}
