#include <algorithm>
#include <atomic>
#include <gtest/gtest.h>
#include <l2encdec.h>

static std::vector<unsigned char> make_input()
{
    return {'T', 'E', 'S', 'T', '1', '2', '3'};
}

static std::vector<unsigned char> make_incompressible_input(size_t size)
{
    std::vector<unsigned char> data(size);
    uint32_t state = 12345;
    for (auto &byte : data)
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        byte = static_cast<unsigned char>(state);
    }
    return data;
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
    std::vector<unsigned char> input = {'B', 'L', 'O', 'W', 'F', 'I', 'S', 'H',
                                        'T', 'E', 'S', 'T', 'D', 'A', 'T', 'A', '!'};
    std::vector<unsigned char> enc, dec;

    ASSERT_EQ(l2encdec::encode(input, enc, 211, "", false),
              l2encdec::EncodeResult::SUCCESS);

    ASSERT_EQ(enc.size(), 28 + input.size() + 20);
    EXPECT_FALSE(std::equal(input.begin(), input.begin() + 16, enc.begin() + 28));

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

TEST(L2EncodeDecode, BlowfishRejectsOversizedKey)
{
    l2encdec::Params params{};
    ASSERT_TRUE(l2encdec::init_params(params, 211));
    params.blowfish_key = std::string(56, 'a');
    params.skip_header = true;
    params.skip_tail = true;
    std::vector<unsigned char> input(8), output;
    EXPECT_EQ(l2encdec::encode(input, output, params), l2encdec::EncodeResult::ENCRYPTION_FAILED);
    EXPECT_EQ(l2encdec::decode(input, output, params), l2encdec::DecodeResult::DECRYPTION_FAILED);
}

TEST(L2Encode, HeaderAndTailApplied)
{
    auto input = make_input();
    std::vector<unsigned char> enc;

    ASSERT_EQ(l2encdec::encode(input, enc, 111, "", false),
              l2encdec::EncodeResult::SUCCESS);

    int tail_size = 20;
    const std::string header = "Lineage2Ver111";
    ASSERT_GE(enc.size(), header.size() * 2 + tail_size);

    for (size_t i = 0; i < header.size(); ++i)
    {
        EXPECT_EQ(enc[i * 2], static_cast<unsigned char>(header[i]));
        EXPECT_EQ(enc[i * 2 + 1], 0);
    }
}

TEST(L2EncodeDecode, NoneTypePassthrough)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;

    l2encdec::Params params{};
    params.type = l2encdec::Type::NONE;
    params.protocol = 111;

    ASSERT_EQ(l2encdec::encode(input, enc, params), l2encdec::EncodeResult::SUCCESS);
    ASSERT_EQ(l2encdec::decode(enc, dec, params), l2encdec::DecodeResult::SUCCESS);
    EXPECT_EQ(dec, input);
}

TEST(L2DecodeFail, TooSmallInput)
{
    std::vector<unsigned char> input = {1, 2};
    std::vector<unsigned char> out;

    EXPECT_EQ(l2encdec::decode(input, out, 111, "", false),
              l2encdec::DecodeResult::INVALID_TYPE);
}

TEST(L2Encode, LegacyRsaFlagIgnoredOnEncode)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;
    ASSERT_EQ(l2encdec::encode(input, enc, 413, "", true), l2encdec::EncodeResult::SUCCESS);
    ASSERT_EQ(l2encdec::decode(enc, dec, 413), l2encdec::DecodeResult::SUCCESS);
    EXPECT_EQ(dec, input);
}

TEST(L2EncodeDecode, ChecksumOfEncodedOutputVerifies)
{
    auto input = make_input();
    std::vector<unsigned char> enc;
    for (int protocol : {111, 120, 211, 413})
    {
        ASSERT_EQ(l2encdec::encode(input, enc, protocol), l2encdec::EncodeResult::SUCCESS);
        EXPECT_EQ(l2encdec::verify_checksum(enc), l2encdec::ChecksumResult::SUCCESS) << protocol;
    }
}

TEST(L2Params, DefaultsAreInitialized)
{
    l2encdec::Params params;
    EXPECT_EQ(params.type, l2encdec::Type::NONE);
    EXPECT_EQ(params.protocol, 0);
    EXPECT_EQ(params.xor_key, 0);
    EXPECT_EQ(params.xor_start_position, 0);
}

TEST(L2Progress, RsaReportsMonotonicProgressToCompletion)
{
    const auto input = make_incompressible_input(4000); // ~33 RSA blocks
    l2encdec::Params params{};
    ASSERT_TRUE(l2encdec::init_params(params, 413));

    for (bool encoding : {true, false})
    {
        SCOPED_TRACE(encoding ? "encode" : "decode");
        std::vector<unsigned char> enc, dec;
        if (!encoding)
            ASSERT_EQ(l2encdec::encode(input, enc, params), l2encdec::EncodeResult::SUCCESS);

        std::vector<std::pair<size_t, size_t>> calls;
        std::atomic<bool> overlapping{false}, inside{false};
        auto progress = [&](size_t done, size_t total)
        {
            if (inside.exchange(true)) overlapping = true;
            calls.emplace_back(done, total);
            inside = false;
            return true;
        };

        if (encoding)
            ASSERT_EQ(l2encdec::encode(input, enc, params, progress), l2encdec::EncodeResult::SUCCESS);
        else
        {
            ASSERT_EQ(l2encdec::decode(enc, dec, params, progress), l2encdec::DecodeResult::SUCCESS);
            EXPECT_EQ(dec, input);
        }

        ASSERT_GE(calls.size(), 2u);
        EXPECT_FALSE(overlapping);
        const size_t total = calls.front().second;
        EXPECT_GT(total, 1u);
        EXPECT_EQ(calls.front().first, 0u);
        EXPECT_EQ(calls.back(), std::make_pair(total, total));
        for (size_t i = 1; i < calls.size(); ++i)
        {
            EXPECT_GE(calls[i].first, calls[i - 1].first);
            EXPECT_EQ(calls[i].second, total);
        }
    }
}

TEST(L2Progress, CancelStopsAndLeavesOutputUntouched)
{
    const auto input = make_incompressible_input(4000);
    l2encdec::Params params{};
    ASSERT_TRUE(l2encdec::init_params(params, 413));

    std::vector<unsigned char> enc;
    ASSERT_EQ(l2encdec::encode(input, enc, params), l2encdec::EncodeResult::SUCCESS);

    for (size_t cancel_at : {0u, 5u})
    {
        SCOPED_TRACE(cancel_at);
        size_t max_done = 0;
        auto cancel = [&](size_t done, size_t)
        { max_done = std::max(max_done, done); return done < cancel_at; };

        std::vector<unsigned char> out{'x'};
        EXPECT_EQ(l2encdec::encode(input, out, params, cancel), l2encdec::EncodeResult::CANCELLED);
        EXPECT_EQ(out, std::vector<unsigned char>{'x'});

        EXPECT_EQ(l2encdec::decode(enc, out, params, cancel), l2encdec::DecodeResult::CANCELLED);
        EXPECT_EQ(out, std::vector<unsigned char>{'x'});
        EXPECT_LT(max_done, 33u); // stopped well before the end
    }
}

TEST(L2Progress, NotCalledForFastTypes)
{
    auto input = make_input();
    std::vector<unsigned char> enc;
    l2encdec::Params params{};
    ASSERT_TRUE(l2encdec::init_params(params, 111));
    bool called = false;
    ASSERT_EQ(l2encdec::encode(input, enc, params, [&](size_t, size_t)
                               { called = true; return false; }),
              l2encdec::EncodeResult::SUCCESS);
    EXPECT_FALSE(called);
}

TEST(L2Span, AcceptsSubspanWithoutCopy)
{
    auto input = make_input();
    std::vector<unsigned char> enc, dec;
    ASSERT_EQ(l2encdec::encode(input, enc, 111), l2encdec::EncodeResult::SUCCESS);

    std::vector<unsigned char> buffer(10, 0xEE);
    buffer.insert(buffer.end(), enc.begin(), enc.end());
    buffer.insert(buffer.end(), 10, 0xEE);
    std::span<const unsigned char> file(buffer.data() + 10, enc.size());

    EXPECT_EQ(l2encdec::verify_checksum(file), l2encdec::ChecksumResult::SUCCESS);
    ASSERT_EQ(l2encdec::decode(file, dec, 111), l2encdec::DecodeResult::SUCCESS);
    EXPECT_EQ(dec, input);
}
