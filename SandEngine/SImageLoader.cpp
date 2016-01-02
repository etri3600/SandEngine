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
	if (pTexture)
	{

		ILuint imageId;
		ilGenImages(1, &imageId);
		ilBindImage(imageId);

		if (ilLoadImage((DirPath + file).c_str()))
		{
			ILinfo info = {};
			iluGetImageInfo(&info);

			pTexture->MipTextures.reserve(info.NumMips);
			pTexture->eTextureLayout = STexture::TextureLayout::TL_TEX_2D;
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
			pTexture->eTextureFormat = eFormat;
			pTexture->MipLevels = info.NumMips;

			for (unsigned int i = 0;i < info.NumMips; ++i)
			{
				pTexture->MipTextures[i]->Width = info.Width;
				pTexture->MipTextures[i]->Height = info.Height;
				pTexture->MipTextures[i]->Size = info.SizeOfData;
				pTexture->MipTextures[i]->pTexData = new unsigned char[info.SizeOfData];
				memcpy(pTexture->MipTextures[i]->pTexData, ilGetData(), info.SizeOfData);
			}
		}

		ilBindImage(0);
	}

	return true;
}