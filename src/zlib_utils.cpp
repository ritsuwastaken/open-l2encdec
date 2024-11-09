#include "zlib_utils.h"
#include <miniz.h>

const size_t COMPRESSED_HEADER_SIZE = 4;
const size_t INFLATE_CHUNK_SIZE = 1024 * 16;
const size_t DEFLATE_CHUNK_SIZE = 1024 * 1024;

int ZlibUtils::unpack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer)
{
    size_t compressed_size = input_buffer.size() - COMPRESSED_HEADER_SIZE;
    if (compressed_size <= 0)
    {
        return -1;
    }

    int expected_decompressed_size = *(reinterpret_cast<const int *>(input_buffer.data()));

    tinfl_decompressor decomp;
    tinfl_init(&decomp);
    output_buffer.clear();
    size_t in_pos = COMPRESSED_HEADER_SIZE;
    size_t out_pos = 0;

    while (in_pos < input_buffer.size())
    {
        output_buffer.resize(out_pos + INFLATE_CHUNK_SIZE);
        size_t in_bytes = input_buffer.size() - in_pos;
        size_t out_bytes = output_buffer.size() - out_pos;
        tinfl_status status = tinfl_decompress(&decomp,
                                               input_buffer.data() + in_pos, &in_bytes,
                                               output_buffer.data(), output_buffer.data() + out_pos, &out_bytes,
                                               TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | TINFL_FLAG_PARSE_ZLIB_HEADER);

        in_pos += in_bytes;
        out_pos += out_bytes;

        if (status == TINFL_STATUS_DONE)
        {
            output_buffer.resize(out_pos);
            break;
        }
        else if (status < 0)
        {
            return -1;
        }
    }

    if (out_pos != expected_decompressed_size)
        return -1;

    return 0;
}

int ZlibUtils::pack(const std::vector<unsigned char> &input_buffer, std::vector<unsigned char> &output_buffer)
{
    size_t uncompressed_size = input_buffer.size();
    output_buffer.clear();
    output_buffer.reserve(uncompressed_size);
    output_buffer.insert(output_buffer.end(),
                         reinterpret_cast<const unsigned char *>(&uncompressed_size),
                         reinterpret_cast<const unsigned char *>(&uncompressed_size) + sizeof(int));

    mz_stream stream = {};
    int status = mz_deflateInit(&stream, MZ_BEST_COMPRESSION);
    if (status != MZ_OK)
        return -1;

    const unsigned char *in = input_buffer.data();
    std::vector<unsigned char> out(DEFLATE_CHUNK_SIZE);
    size_t input_pos = 0;

    do
    {
        stream.avail_in = static_cast<unsigned int>(std::min(DEFLATE_CHUNK_SIZE, input_buffer.size() - input_pos));
        stream.next_in = const_cast<unsigned char *>(&in[input_pos]);

        do
        {
            stream.avail_out = DEFLATE_CHUNK_SIZE;
            stream.next_out = out.data();

            bool is_last_chunk = (input_pos + stream.avail_in == input_buffer.size());
            status = mz_deflate(&stream, is_last_chunk ? MZ_FINISH : MZ_NO_FLUSH);
            if (status != MZ_OK && status != MZ_STREAM_END)
            {
                mz_deflateEnd(&stream);
                return -1;
            }

            int have = DEFLATE_CHUNK_SIZE - stream.avail_out;
            output_buffer.insert(output_buffer.end(), out.data(), out.data() + have);
        } while (stream.avail_out == 0);

        input_pos += DEFLATE_CHUNK_SIZE - stream.avail_in;
    } while (input_pos < input_buffer.size());

    mz_deflateEnd(&stream);
    if (status != MZ_STREAM_END)
        return -1;

    return 0;
}

uint32_t ZlibUtils::checksum(const std::vector<unsigned char> &buffer, uint32_t checksum)
{
    return mz_crc32(checksum, buffer.data(), buffer.size());
}
