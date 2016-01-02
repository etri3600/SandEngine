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
	unsigned int MipLevels = 0;
	unsigned int CurrentMipLevel = 0;
	std::vector<SMipTexture*> MipTextures;

	SMipTexture GetCurrentMipTexture() { return MipTextures.size() > CurrentMipLevel ? *MipTextures[CurrentMipLevel] : SMipTexture(); }
	unsigned int GetTextureFormat() const;
};