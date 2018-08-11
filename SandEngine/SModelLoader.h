#pragma once

#include "SLoader.h"

#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include "assimp\Importer.hpp"

#include <map>
#include <vector>

#include "SMath.h"
#include "SModelStruct.h"
#include "SAnimation.h"
#include "SImageLoader.h"

class SModel;
class SAnimation;
class SImageLoader;
class SModelLoader : public SLoader
{
public:
	bool Initialize();

	SModel LoadModelFromFile(const wchar_t* file);


private:
	void	LoadMeshes(SModel* model, const aiScene* pScene);
	void	LoadBones(SModel* model, aiMesh* pMesh, unsigned int meshIndex);
	void	LoadTextures(SModel* model, std::wstring& directory, const aiScene* pScene);

	void	LoadAnimation(SAnimation* animation, const aiScene* pScene);

	SBoneNode*	CreateBoneTree(SAnimation* animation, SBoneNode* pBone, aiNode* node);

	SMatrix MatrixFromAI(const aiMatrix4x4& aiMatrix);
	SVector3 Vector3FromAI(const aiVector3D& aiVector3);
	SVector4 Vector4FromAI(const aiColor4D& aiColor4);
	SQuaternion QuatFromAI(const aiQuaternion& aiQuat);
	std::vector<SAnimTimelineVector> ATVFromAI(const aiVectorKey* pVectorKey, const unsigned int num);
	std::vector<SAnimTimelineQuat> ATQFromAI(const aiQuatKey* pQuatKey, const unsigned int num);

protected:
	SImageLoader* m_ImageLoader;
};