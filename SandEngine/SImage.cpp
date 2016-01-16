#include "SImage.h"

#include "SGraphics.h"

#include "SDirectX12.h"

STexture::~STexture()
{
	for (auto MipTexture : MipTextures)
	{
		delete MipTexture->pTexData;
		MipTexture->pTexData = nullptr;
	}
	MipTextures.clear();
}

unsigned int STexture::GetTextureFormat() const
{
	unsigned int format = 0;
	if (SGraphics::s_eGraphicInterface == GraphicsInterfaceEnum::GI_DX_12)
	{
		switch (eTextureFormat)
		{
		case TextureFormat::TF_RGB:
		case TextureFormat::TF_BGR: format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case TextureFormat::TF_RGBA:
		case TextureFormat::TF_BGRA: format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		}
	}
	else
	{

	}

	return 0;
}

unsigned int STexture::GetTexturePixelSize() const
{
	unsigned int size = 0;
	switch (eTextureFormat)
	{
	case TextureFormat::TF_RGB:
	case TextureFormat::TF_BGR: size = 4 * 3;
		break;
	case TextureFormat::TF_RGBA:
	case TextureFormat::TF_BGRA: size = 4 * 4;
		break;
	}

	return size;
}
