#include "webp.h"
#include <webp/encode.h>
#include <string.h>

void compress_webp(const unsigned char *pic, int Width, int Height, std::vector<uint8_t> &CompressedData)
{
    bool hasTransparency = false;
    const unsigned char *p = pic + 3;
    for (int i = 0; i < Width * Height; i++, p += 4)
    {
        if (*p != 255)
        {
            hasTransparency = true;
            break;
        }
    }

    size_t webp_size = 0;
    uint8_t *webp_data = nullptr;

    if (!hasTransparency)
    {
        std::vector<unsigned char> rgb_data(Width * Height * 3);
        const unsigned char *s = pic;
        unsigned char *d = rgb_data.data();
        for (int i = 0; i < Width * Height; i++)
        {
            *d++ = *s++; // R
            *d++ = *s++; // G
            *d++ = *s++; // B
            s++;         // Skip alpha
        }

        webp_size = WebPEncodeRGB(rgb_data.data(), Width, Height, Width * 3, 75, &webp_data);
    }
    else
    {
        webp_size = WebPEncodeRGBA(pic, Width, Height, Width * 4, 75, &webp_data);
    }

    if (webp_size > 0 && webp_data != nullptr)
    {
        CompressedData.resize(webp_size);
        memcpy(CompressedData.data(), webp_data, webp_size);
        WebPFree(webp_data);
    }
}

static void decompress_dxt1_block(const unsigned char *block, unsigned char *output, int stride)
{
    uint16_t c0 = (block[0]) | (block[1] << 8);
    uint16_t c1 = (block[2]) | (block[3] << 8);

    unsigned char colors[4][4];

    colors[0][0] = ((c0 >> 11) & 31) * 255 / 31; // R
    colors[0][1] = ((c0 >> 5) & 63) * 255 / 63;  // G
    colors[0][2] = (c0 & 31) * 255 / 31;         // B
    colors[0][3] = 255;                          // A

    colors[1][0] = ((c1 >> 11) & 31) * 255 / 31;
    colors[1][1] = ((c1 >> 5) & 63) * 255 / 63;
    colors[1][2] = (c1 & 31) * 255 / 31;
    colors[1][3] = 255;

    if (c0 > c1)
    {
        for (int i = 0; i < 3; i++)
        {
            colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
            colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
        }
        colors[2][3] = colors[3][3] = 255;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            colors[2][i] = (colors[0][i] + colors[1][i]) / 2;
            colors[3][i] = 0;
        }
        colors[2][3] = 255;
        colors[3][3] = 0;
    }

    uint32_t indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int idx = (indices >> (2 * (y * 4 + x))) & 3;
            unsigned char *pixel = output + (y * stride + x * 4);
            pixel[0] = colors[idx][0]; // R
            pixel[1] = colors[idx][1]; // G
            pixel[2] = colors[idx][2]; // B
            pixel[3] = colors[idx][3]; // A
        }
    }
}

static void decompress_dxt3_block(const unsigned char *block, unsigned char *output, int stride)
{
    uint64_t alphaValues =
        static_cast<uint64_t>(block[0]) |
        (static_cast<uint64_t>(block[1]) << 8) |
        (static_cast<uint64_t>(block[2]) << 16) |
        (static_cast<uint64_t>(block[3]) << 24) |
        (static_cast<uint64_t>(block[4]) << 32) |
        (static_cast<uint64_t>(block[5]) << 40) |
        (static_cast<uint64_t>(block[6]) << 48) |
        (static_cast<uint64_t>(block[7]) << 56);

    uint16_t c0 = (block[8]) | (block[9] << 8);
    uint16_t c1 = (block[10]) | (block[11] << 8);

    unsigned char colors[4][4];

    colors[0][0] = ((c0 >> 11) & 31) * 255 / 31; // R
    colors[0][1] = ((c0 >> 5) & 63) * 255 / 63;  // G
    colors[0][2] = (c0 & 31) * 255 / 31;         // B

    colors[1][0] = ((c1 >> 11) & 31) * 255 / 31;
    colors[1][1] = ((c1 >> 5) & 63) * 255 / 63;
    colors[1][2] = (c1 & 31) * 255 / 31;

    for (int i = 0; i < 3; i++)
    {
        colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
        colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
    }

    uint32_t indices = block[12] | (block[13] << 8) | (block[14] << 16) | (block[15] << 24);

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int idx = (indices >> (2 * (y * 4 + x))) & 3;
            unsigned char *pixel = output + (y * stride + x * 4);

            int alphaIndex = y * 4 + x;
            int alphaShift = alphaIndex * 4;
            int alpha = (alphaValues >> alphaShift) & 0xF;

            pixel[0] = colors[idx][0]; // R
            pixel[1] = colors[idx][1]; // G
            pixel[2] = colors[idx][2]; // B
            pixel[3] = alpha * 17;     // Scale 4-bit (0-15) to 8-bit (0-255)
        }
    }
}

void webp::from_dxt1(const std::vector<unsigned char> &compressed, int USize, int VSize, std::vector<unsigned char> &output)
{
    int pixelDataSize = USize * VSize * 4;
    unsigned char *decompressed = new unsigned char[pixelDataSize];

    int blocksWide = (USize + 3) / 4;
    int blocksHigh = (VSize + 3) / 4;

    size_t compressedIndex = 0;
    for (int by = 0; by < blocksHigh; by++)
    {
        for (int bx = 0; bx < blocksWide; bx++)
        {
            unsigned char *dstBlock = decompressed + (by * 4 * USize + bx * 4) * 4;
            decompress_dxt1_block(&compressed[compressedIndex], dstBlock, USize * 4);
            compressedIndex += 8;
        }
    }

    std::vector<uint8_t> CompressedData;
    compress_webp(decompressed, USize, VSize, CompressedData);
    delete[] decompressed;

    if (!CompressedData.empty())
        output.insert(output.end(), CompressedData.begin(), CompressedData.end());
}

void webp::from_dxt3(const std::vector<unsigned char> &compressed, int USize, int VSize, std::vector<unsigned char> &output)
{
    int pixelDataSize = USize * VSize * 4;
    unsigned char *decompressed = new unsigned char[pixelDataSize];

    int blocksWide = (USize + 3) / 4;
    int blocksHigh = (VSize + 3) / 4;

    size_t compressedIndex = 0;
    for (int by = 0; by < blocksHigh; by++)
    {
        for (int bx = 0; bx < blocksWide; bx++)
        {
            unsigned char *dstBlock = decompressed + (by * 4 * USize + bx * 4) * 4;
            decompress_dxt3_block(&compressed[compressedIndex], dstBlock, USize * 4);
            compressedIndex += 16;
        }
    }

    std::vector<uint8_t> CompressedData;
    compress_webp(decompressed, USize, VSize, CompressedData);
    delete[] decompressed;

    if (!CompressedData.empty())
        output.insert(output.end(), CompressedData.begin(), CompressedData.end());
}
