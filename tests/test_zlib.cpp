#include "zlib_utils.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(ZlibUtils, PackUnpackRoundTrip)
{
    std::string s = "This is a test string to compress and decompress with zlib-ng.";
    std::vector<unsigned char> input(s.begin(), s.end());
    std::vector<unsigned char> packed, unpacked;

    int pack_status = zlib_utils::pack(input, packed);
    ASSERT_EQ(pack_status, 0);
    ASSERT_GT(packed.size(), 4);

    int unpack_status = zlib_utils::unpack(packed, unpacked);
    ASSERT_EQ(unpack_status, 0);
    std::string out(unpacked.begin(), unpacked.end());
    EXPECT_EQ(out, s);
}

TEST(ZlibUtils, EmptyRoundTrip)
{
    std::vector<unsigned char> input, packed, unpacked;
    ASSERT_EQ(zlib_utils::pack(input, packed), 0);
    ASSERT_EQ(zlib_utils::unpack(packed, unpacked), 0);
    EXPECT_TRUE(unpacked.empty());
}

TEST(ZlibUtils, LargeIncompressibleRoundTrip)
{
    std::vector<unsigned char> input(3 * 1024 * 1024 + 17);
    uint32_t state = 12345;
    for (auto &byte : input)
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        byte = static_cast<unsigned char>(state);
    }

    for (size_t size : {input.size(), size_t{1024 * 1024}})
    {
        SCOPED_TRACE(size);
        input.resize(size);
        std::vector<unsigned char> packed, unpacked;
        ASSERT_EQ(zlib_utils::pack(input, packed), 0);
        ASSERT_EQ(zlib_utils::unpack(packed, unpacked), 0);
        EXPECT_EQ(unpacked, input);
    }
}

TEST(ZlibUtils, RejectsTruncatedStream)
{
    std::vector<unsigned char> input(100000, 'x'), packed, unpacked;
    ASSERT_EQ(zlib_utils::pack(input, packed), 0);
    packed.resize(packed.size() - 4);
    EXPECT_NE(zlib_utils::unpack(packed, unpacked), 0);
}

TEST(ZlibUtils, RejectsIncorrectUncompressedSize)
{
    std::vector<unsigned char> input(100000, 'x'), packed, unpacked;
    ASSERT_EQ(zlib_utils::pack(input, packed), 0);
    ++packed[0];
    EXPECT_NE(zlib_utils::unpack(packed, unpacked), 0);
}

TEST(ZlibUtils, RejectsZeroSizeForNonemptyStream)
{
    std::vector<unsigned char> input{'x'}, packed, unpacked;
    ASSERT_EQ(zlib_utils::pack(input, packed), 0);
    packed[0] = packed[1] = packed[2] = packed[3] = 0;
    EXPECT_NE(zlib_utils::unpack(packed, unpacked), 0);
}

TEST(ZlibUtils, StandardAndIncrementalCrc32)
{
    std::vector<unsigned char> input{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    EXPECT_EQ(zlib_utils::checksum(input), 0xcbf43926u);
    std::vector<unsigned char> first(input.begin(), input.begin() + 4);
    std::vector<unsigned char> second(input.begin() + 4, input.end());
    EXPECT_EQ(zlib_utils::checksum(second, zlib_utils::checksum(first)), 0xcbf43926u);
}

TEST(ZlibUtils, ChecksumUsesOnlyTheRequestedSlice)
{
    const std::vector<unsigned char> input{'?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!'};
    EXPECT_EQ(zlib_utils::checksum(std::span(input).subspan(1, 9)), 0xcbf43926u);
}
