#pragma once

#include "Loader.h"

#include "IL\ilu.h"

#include <string>

#include "Assets/Image.h"

class SImageLoader : public SLoader
{
public:
	bool Initialize();

	bool LoadImageFromFile(const wchar_t* file, SImage* pImage);
	bool LoadTextureFromFile(const wchar_t* file, STexture* pTexture);
};