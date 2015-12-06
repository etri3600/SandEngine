#include "SImageLoader.h"

#include <cassert>

namespace {
	const std::wstring DirPath = LR"(..\Model\)";
}

bool SImageLoader::Initialize()
{
	ilInit();
	iluInit();

	return true;
}

bool SImageLoader::LoadImageFromFile(const wchar_t* file, SImage* pImage)
{
	//ILuint image = iluLoadImage((DirPath+file).c_str());
	//ILubyte* data = ilGetData();
	//ILinfo info;
	//iluGetImageInfo(&info);

	return true;
}

bool SImageLoader::LoadTextureFromFile(const wchar_t* file, STexture* pTexture)
{
	ILuint imageId;
	ilGenImages(1, &imageId);
	ilBindImage(imageId);
	
	if (ilLoadImage((DirPath + file).c_str()))
	{
		ILinfo info = {};
		iluGetImageInfo(&info);

		STexture2D* pTexture2D = static_cast<STexture2D*>(pTexture);
		if (pTexture2D)
		{
			pTexture2D->eTextureLayout = STexture::TextureLayout::TL_TEX_2D;
			STexture::TextureFormat eFormat;
			switch (info.Format)
			{
			case IL_RGB: eFormat = STexture::TextureFormat::TF_RGB;
				break;
			case IL_RGBA: eFormat = STexture::TextureFormat::TF_RGBA;
				break;
			case IL_BGR: eFormat = STexture::TextureFormat::TF_BGR;
				break;
			case IL_BGRA: eFormat = STexture::TextureFormat::TF_BGRA;
				break;
			default:
				assert(false);
			}
			pTexture2D->eTextureFormat = eFormat;
			pTexture2D->MipLevels = info.NumMips;
			pTexture2D->Width = info.Width;
			pTexture2D->Height = info.Height;
			pTexture2D->Size = info.SizeOfData;
			pTexture2D->pTexData = new unsigned char[pTexture2D->Size];
			memcpy(pTexture2D->pTexData, ilGetData(), pTexture2D->Size);
		}
	}

	ilBindImage(0);

	return true;
}