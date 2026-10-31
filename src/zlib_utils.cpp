#include "zlib_utils.h"
#include <algorithm>
#include <cstring>
#include <limits>
#include <zlib-ng.h>

namespace
{
constexpr size_t COMPRESSED_HEADER_SIZE = 4;
constexpr size_t INFLATE_CHUNK_SIZE = 1024 * 16;
constexpr size_t DEFLATE_CHUNK_SIZE = 1024 * 1024;
} // namespace

int zlib_utils::unpack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer)
{
    if (input_buffer.size() < COMPRESSED_HEADER_SIZE)
        return -1;

    uint32_t expected_decompressed_size = 0;
    std::memcpy(&expected_decompressed_size, input_buffer.data(), sizeof(expected_decompressed_size));

    zng_stream stream{};
    if (zng_inflateInit(&stream) != Z_OK)
        return -1;

    output_buffer.clear();
    size_t in_pos = COMPRESSED_HEADER_SIZE;
    size_t out_pos = 0;
    int status;

    do
    {
        if (stream.avail_in == 0)
        {
            stream.avail_in = static_cast<uint32_t>(std::min(INFLATE_CHUNK_SIZE, input_buffer.size() - in_pos));
            stream.next_in = input_buffer.data() + in_pos;
            in_pos += stream.avail_in;
        }

        output_buffer.resize(out_pos + INFLATE_CHUNK_SIZE);
        stream.next_out = output_buffer.data() + out_pos;
        stream.avail_out = INFLATE_CHUNK_SIZE;
        status = zng_inflate(&stream, Z_NO_FLUSH);
        out_pos += INFLATE_CHUNK_SIZE - stream.avail_out;

        if ((status != Z_OK && status != Z_STREAM_END) || out_pos > expected_decompressed_size)
        {
            zng_inflateEnd(&stream);
            return -1;
        }
    } while (status != Z_STREAM_END);

    zng_inflateEnd(&stream);
    output_buffer.resize(out_pos);

    if (out_pos != static_cast<size_t>(expected_decompressed_size))
        return -1;

    return 0;
}

int zlib_utils::pack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer)
{
    if (input_buffer.size() > std::numeric_limits<uint32_t>::max())
        return -1;

    uint32_t uncompressed_size = static_cast<uint32_t>(input_buffer.size());
    output_buffer.clear();
    output_buffer.reserve(uncompressed_size);
    output_buffer.insert(output_buffer.end(),
                         reinterpret_cast<const unsigned char *>(&uncompressed_size),
                         reinterpret_cast<const unsigned char *>(&uncompressed_size) + sizeof(uncompressed_size));

    zng_stream stream = {};
    int status = zng_deflateInit(&stream, Z_BEST_COMPRESSION);
    if (status != Z_OK)
        return -1;

    std::vector<unsigned char> out(DEFLATE_CHUNK_SIZE);
    size_t input_pos = 0;

    do
    {
        size_t chunk_size = std::min(DEFLATE_CHUNK_SIZE, input_buffer.size() - input_pos);
        stream.avail_in = static_cast<uint32_t>(chunk_size);
        stream.next_in = input_buffer.empty() ? nullptr : input_buffer.data() + input_pos;
        int flush = input_pos + chunk_size == input_buffer.size() ? Z_FINISH : Z_NO_FLUSH;

        do
        {
            stream.avail_out = DEFLATE_CHUNK_SIZE;
            stream.next_out = out.data();

            status = zng_deflate(&stream, flush);
            if (status != Z_OK && status != Z_STREAM_END)
            {
                zng_deflateEnd(&stream);
                return -1;
            }

            int have = DEFLATE_CHUNK_SIZE - stream.avail_out;
            output_buffer.insert(output_buffer.end(), out.data(), out.data() + have);
        } while (stream.avail_out == 0);

        input_pos += chunk_size - stream.avail_in;
    } while (input_pos < input_buffer.size());

    zng_deflateEnd(&stream);
    if (status != Z_STREAM_END)
        return -1;

    return 0;
}

uint32_t zlib_utils::checksum(const std::vector<unsigned char> &buffer, uint32_t checksum)
{
    return zng_crc32_z(checksum, buffer.data(), buffer.size());
}
