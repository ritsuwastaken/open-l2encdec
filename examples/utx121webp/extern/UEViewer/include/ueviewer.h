#ifndef UEVIEWER_H
#define UEVIEWER_H

#include <string>
#include <vector>

// Partly ETexturePixelFormat
enum class TextureFormat
{
    TPF_DXT1 = 6,
    TPF_DXT3 = 7,
};

struct TextureData
{
    std::string name;
    int width;
    int height;
    std::vector<unsigned char> data;
    TextureFormat format;
};

class UEViewer
{
public:
    static UEViewer &getInstance();

    void initialize();
    bool extract_textures(const std::string &filename, const std::vector<unsigned char> &data, std::vector<TextureData> &textures);

    UEViewer(const UEViewer &) = delete;
    UEViewer &operator=(const UEViewer &) = delete;
    UEViewer(UEViewer &&) = delete;
    UEViewer &operator=(UEViewer &&) = delete;

private:
    UEViewer() = default;
    ~UEViewer() = default;
};

#endif // UEVIEWER_H
