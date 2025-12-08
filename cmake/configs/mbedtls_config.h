#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#define MBEDTLS_NO_PLATFORM_ENTROPY

#define MBEDTLS_BIGNUM_C
#if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER <= 1900)
#define MBEDTLS_PLATFORM_C
#endif

#endif // MBEDTLS_CONFIG_H
