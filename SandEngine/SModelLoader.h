#pragma once

#include "SLoader.h"

#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include "assimp\Importer.hpp"

#include "SUtils.h"
#include "SImageLoader.h"

class SModel;
class SModelLoader : public SLoader
{
public:
	bool Initialize();

	SModel	LoadModelFromFile(const wchar_t* file);

private:
	void	LoadModelFromNode(SModel* model , aiNode* node);

protected:
	SImageLoader* m_ImageLoader;
};