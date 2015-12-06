#include "SImage.h"

#include "SGraphics.h"

#include "SDirectX12.h"

STexture::~STexture()
{
	delete[] pTexData;
	pTexData = nullptr;
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

STexture2D::~STexture2D()
{

}