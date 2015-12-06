#pragma once

#include "IL\ilu.h"

#include <string>

#include "SImage.h"

class SImageLoader
{
public:
	bool Initialize();

	bool LoadImageFromFile(const wchar_t* file, SImage* pImage);
	bool LoadTextureFromFile(const wchar_t* file, STexture* pTexture);
};