#include <gtest/gtest.h>
#include <l2encdec.h>

TEST(L2InitParams, InvalidProtocol)
{
    l2encdec::Params p{};
    EXPECT_FALSE(l2encdec::init_params(p, 999, "", false));
}

TEST(L2InitParams, XORFilenameRequiresFilename)
{
    l2encdec::Params p{};
    EXPECT_FALSE(l2encdec::init_params(p, 121, "", false));
}

TEST(L2InitParams, XORBasic)
{
    l2encdec::Params p{};
    ASSERT_TRUE(l2encdec::init_params(p, 111, "", false));
    EXPECT_EQ(p.type, l2encdec::Type::XOR);
    EXPECT_EQ(p.xor_key, 0xAC);
    EXPECT_EQ(p.protocol, 111);
}

TEST(L2InitParams, RSAOverridesLegacy)
{
    l2encdec::Params p{};
    ASSERT_TRUE(l2encdec::init_params(p, 413, "", false));
    EXPECT_EQ(p.type, l2encdec::Type::RSA);
    EXPECT_EQ(p.rsa_private_exponent, "1d");
}

TEST(L2InitParams, RSAUsesLegacy)
{
    l2encdec::Params p{};
    ASSERT_TRUE(l2encdec::init_params(p, 413, "", true));
    EXPECT_EQ(p.type, l2encdec::Type::RSA);
    EXPECT_EQ(p.rsa_private_exponent, "35");
}
