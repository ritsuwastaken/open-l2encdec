#include "utils.h"
#include "zlib_utils.h"
#include <gtest/gtest.h>
#include <l2encdec.h>

constexpr size_t CRC32_OFFSET = 12;
constexpr size_t TAIL_SIZE = 20;

TEST(L2Checksum, VerifySuccess)
{
    std::vector<unsigned char> data = {'A', 'B', 'C'};

    std::string tail = utils::make_tail(
        zlib_utils::checksum(data),
        CRC32_OFFSET,
        TAIL_SIZE);

    utils::add_tail(data, tail);

    EXPECT_EQ(
        l2encdec::verify_checksum(data),
        l2encdec::ChecksumResult::SUCCESS);
}

TEST(L2Checksum, VerifyMismatch)
{
    std::vector<unsigned char> data = {'A', 'B', 'C'};
    std::string tail = utils::make_tail(
        zlib_utils::checksum(data),
        CRC32_OFFSET,
        TAIL_SIZE);

    utils::add_tail(data, tail);

    const size_t base = data.size() - TAIL_SIZE + CRC32_OFFSET;
    data[base + 0] ^= 0xFF;
    data[base + 1] ^= 0xFF;
    data[base + 2] ^= 0xFF;
    data[base + 3] ^= 0xFF;

    EXPECT_EQ(
        l2encdec::verify_checksum(data),
        l2encdec::ChecksumResult::MISMATCH);
}
