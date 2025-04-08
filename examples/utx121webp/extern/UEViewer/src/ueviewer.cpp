#include <ueviewer.h>
#include "Core.h"
#include "UnCore.h"
#include "UnObject.h"
#include "UnMaterial.h"
#include "UnMaterial2.h"
#include "UnPackage.h"

static TextureData extract_texture_data(UTexture *texture)
{
    TextureData result;
    result.name = texture->Name;

    CTextureData texData;
    texData.SetObject(texture);
    if (texture->GetTextureData(texData) && texData.Mips.Num() > 0)
    {
        const CMipMap &mip = texData.Mips[0];
        result.width = mip.USize;
        result.height = mip.VSize;
        result.format = static_cast<TextureFormat>(texData.Format);
        result.data.assign(mip.CompressedData, mip.CompressedData + mip.DataSize);
    }

    return result;
}

UEViewer &UEViewer::getInstance()
{
    static UEViewer instance;
    return instance;
}

void UEViewer::initialize()
{
    RegisterCoreClasses();
    BEGIN_CLASS_TABLE
    REGISTER_MATERIAL_CLASSES
    END_CLASS_TABLE
    REGISTER_MATERIAL_ENUMS
}

bool UEViewer::extract_textures(const std::string &filename, const std::vector<unsigned char> &data, std::vector<TextureData> &textures)
{
    UnPackage *package = UnPackage::LoadPackageFromMemory(filename.c_str(), data, GAME_Lineage2);
    if (!package)
        return false;

    for (int i = 0; i < package->Summary.ExportCount; i++)
    {
        const char *className = package->GetClassNameFor(package->GetExport(i));
        if (strcmp(className, "Texture") != 0)
            continue;

        TextureData texData = extract_texture_data(static_cast<UTexture *>(package->CreateExport(i)));
        textures.push_back(texData);
    }

    return true;
}
