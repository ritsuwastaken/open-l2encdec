#include "zlib_utils.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(ZlibUtils, PackUnpackRoundTrip)
{
    std::string s = "This is a test string to compress and decompress via miniz.";
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
