#include "Image.h"

#include "Graphics/Graphics.h"

#include "Graphics/DirectX12/DirectX12.h"

STexture::~STexture()
{
	for (auto MipTexture : MipTextures)
	{
		delete[] MipTexture->pTexData;
		MipTexture->pTexData = nullptr;
	}
	MipTextures.clear();
}

unsigned int STexture::GetTextureFormat() const
{
	unsigned int format = 0;
	if (SGraphics::s_eGraphicInterface == EGraphicsInterfaceEnum::DX12)
	{
		switch (eTextureFormat)
		{
		case TextureFormat::TF_RGB:
		case TextureFormat::TF_BGR:
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TextureFormat::TF_RGBA:
		case TextureFormat::TF_BGRA: 
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		}
	}
	else
	{

	}

	return format;
}

unsigned int STexture::GetTexelSize() const
{
	return BytesPerPixel;
}
