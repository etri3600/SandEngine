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
		case TextureFormat::TF_BGR: 
			{
				switch (eTexelType)
				{
				case TexelType::TT_INT: format = DXGI_FORMAT_R32G32B32_UINT; break;
				case TexelType::TT_FLOAT: format = DXGI_FORMAT_R32G32B32_FLOAT; break;
				default: format = DXGI_FORMAT_R32G32B32_FLOAT; break;
				}
			}
			break;
		case TextureFormat::TF_RGBA:
		case TextureFormat::TF_BGRA: 
			{
				switch (eTexelType)
				{
				case TexelType::TT_INT: format = DXGI_FORMAT_R32G32B32A32_UINT; break;
				case TexelType::TT_FLOAT: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
				default: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
				}
			}
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
	unsigned int size = 0;
	switch (eTexelType)
	{
	case TexelType::TT_BYTE: size = 1;
		break;
	case TexelType::TT_SHORT: size = 2;
		break;
	case TexelType::TT_INT: size = 4;
		break;
	case TexelType::TT_FLOAT: size = 4;
		break;
	case TexelType::TT_DOUBLE: size = 8;
		break;
	case TexelType::TT_HALF: size = 2;
		break;
	}

	return size;
}
