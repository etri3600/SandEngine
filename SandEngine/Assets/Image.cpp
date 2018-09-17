#include "Image.h"

#include "Graphics/Graphics.h"

STexture::~STexture()
{
	for (auto MipTexture : MipTextures)
	{
		delete[] MipTexture->pTexData;
		MipTexture->pTexData = nullptr;
	}
	MipTextures.clear();
}

EGraphicsFormat STexture::GetTextureFormat() const
{
    EGraphicsFormat format;

    switch (eTextureFormat)
    {
    case TextureFormat::TF_RGB:
    case TextureFormat::TF_BGR:
        format = EGraphicsFormat::FORMAT_R8G8B8A8_UNORM;
        break;
    case TextureFormat::TF_RGBA:
    case TextureFormat::TF_BGRA:
        format = EGraphicsFormat::FORMAT_R8G8B8A8_UNORM;
        break;
    }

	return format;
}

unsigned int STexture::GetTexelSize() const
{
	return BytesPerPixel;
}
