#pragma once

#include <vector>

struct SImage
{

};

struct STexture : public SImage
{
	virtual ~STexture();

	enum class TextureLayout
	{
		TL_TEX_1D,
		TL_TEX_2D,
		TL_TEX_3D,
	};

	enum class TextureFormat
	{
		TF_RGB,
		TF_RGBA,
		TF_BGR,
		TF_BGRA,
	};

	enum class TexelType
	{
		TT_BYTE,
		TT_SHORT,
		TT_INT,
		TT_FLOAT,
		TT_DOUBLE,
		TT_HALF
	};

	struct SMipTexture
	{
		unsigned char* pTexData = nullptr;
		unsigned int Size = 0;
		unsigned int Width = 0;
		unsigned int Height = 0;
		unsigned int Depth = 0;
	};

	TextureLayout eTextureLayout = TextureLayout::TL_TEX_2D;
	TextureFormat eTextureFormat = TextureFormat::TF_RGBA;
	TexelType eTexelType = TexelType::TT_BYTE;
	unsigned int BytesPerPixel = 0;
	unsigned int MipLevels = 0;
	unsigned int CurrentMipLevel = 0;
	bool FlipY = false;
	std::vector<SMipTexture*> MipTextures;

	SMipTexture GetCurrentMipTexture() const { return MipTextures.size() > CurrentMipLevel ? *MipTextures[CurrentMipLevel] : SMipTexture(); }
	unsigned int GetTextureFormat() const;
	unsigned int GetTexelSize() const;
	unsigned int GetWidth() const{
		unsigned int Width = 0;
		switch (eTextureLayout)
		{
		case STexture::TextureLayout::TL_TEX_1D:
			Width = GetCurrentMipTexture().Size;
			break;
		case STexture::TextureLayout::TL_TEX_2D:
		case STexture::TextureLayout::TL_TEX_3D:
			Width = GetCurrentMipTexture().Width;
			break;
		}
		return Width;
	}
	unsigned int GetHeight() const {
		unsigned int Height = 0;
		switch (eTextureLayout)
		{
		case STexture::TextureLayout::TL_TEX_1D:
			Height = 1;
			break;
		case STexture::TextureLayout::TL_TEX_2D:
		case STexture::TextureLayout::TL_TEX_3D:
			Height = GetCurrentMipTexture().Height;
			break;
		}
		return Height;
	}
};