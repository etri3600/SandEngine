#pragma once

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

	unsigned int MipLevels = 0;
	unsigned int Size = 0;
	unsigned char* pTexData = nullptr;
	TextureLayout eTextureLayout = TextureLayout::TL_TEX_2D;
	TextureFormat eTextureFormat = TextureFormat::TF_RGBA;

	unsigned int GetTextureFormat() const;
};

struct STexture2D : public STexture
{
	~STexture2D() override;

	unsigned int Width = 0;
	unsigned int Height = 0;
};