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

			pTexture->MipTextures.resize(info.NumMips == 0 ? 1 : info.NumMips);
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

			STexture::TexelType eType;
			switch (info.Type)
			{
			case IL_BYTE:
			case IL_UNSIGNED_BYTE: eType = STexture::TexelType::TT_BYTE;
				break;
			case IL_SHORT:
			case IL_UNSIGNED_SHORT: eType = STexture::TexelType::TT_SHORT;
				break;
			case IL_INT:
			case IL_UNSIGNED_INT: eType = STexture::TexelType::TT_BYTE;
				break;
			case IL_FLOAT: eType = STexture::TexelType::TT_FLOAT;
				break;
			case IL_DOUBLE: eType = STexture::TexelType::TT_DOUBLE;
				break;
			case IL_HALF: eType = STexture::TexelType::TT_HALF;
				break;
			}
			pTexture->eTexelType = eType;
			
			for (unsigned int i = 0;i < pTexture->MipTextures.size(); ++i)
			{
				pTexture->MipTextures[i] = new STexture::SMipTexture();
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