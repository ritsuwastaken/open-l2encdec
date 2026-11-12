#include "zlib_utils.h"
#include <cstring>
#include <limits>
#include <zlib-ng.h>

namespace
{
constexpr size_t COMPRESSED_HEADER_SIZE = 4;
constexpr uint64_t MAX_DEFLATE_RATIO = 1032;
} // namespace

int zlib_utils::unpack(std::span<const unsigned char> input_buffer, std::vector<unsigned char> &output_buffer)
{
    if (input_buffer.size() < COMPRESSED_HEADER_SIZE)
        return -1;

    uint32_t expected_decompressed_size = 0;
    std::memcpy(&expected_decompressed_size, input_buffer.data(), sizeof(expected_decompressed_size));

    if (expected_decompressed_size > output_buffer.max_size() ||
        expected_decompressed_size > (input_buffer.size() - COMPRESSED_HEADER_SIZE) * MAX_DEFLATE_RATIO)
        return -1;

    output_buffer.resize(expected_decompressed_size ? expected_decompressed_size : 1);
    size_t decompressed_size = output_buffer.size();
    if (zng_uncompress(output_buffer.data(), &decompressed_size,
                       input_buffer.data() + COMPRESSED_HEADER_SIZE,
                       input_buffer.size() - COMPRESSED_HEADER_SIZE) != Z_OK ||
        decompressed_size != expected_decompressed_size)
        return -1;

    output_buffer.resize(decompressed_size);

    return 0;
}

int zlib_utils::pack(std::span<const unsigned char> input_buffer, std::vector<unsigned char> &output_buffer)
{
    if (input_buffer.size() > std::numeric_limits<uint32_t>::max())
        return -1;

    size_t compressed_size = zng_compressBound(input_buffer.size());
    if (compressed_size < input_buffer.size() || compressed_size > output_buffer.max_size() - COMPRESSED_HEADER_SIZE)
        return -1;

    output_buffer.resize(COMPRESSED_HEADER_SIZE + compressed_size);
    uint32_t uncompressed_size = static_cast<uint32_t>(input_buffer.size());
    std::memcpy(output_buffer.data(), &uncompressed_size, sizeof(uncompressed_size));

    if (zng_compress2(output_buffer.data() + COMPRESSED_HEADER_SIZE, &compressed_size,
                      input_buffer.data(), input_buffer.size(), Z_BEST_COMPRESSION) != Z_OK)
        return -1;

    output_buffer.resize(COMPRESSED_HEADER_SIZE + compressed_size);

    return 0;
}

uint32_t zlib_utils::checksum(std::span<const unsigned char> buffer, uint32_t checksum)
{
    return zng_crc32_z(checksum, buffer.data(), buffer.size());
}
