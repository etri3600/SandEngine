#pragma once

#include "SLoader.h"

#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include "assimp\Importer.hpp"

#include <map>
#include <vector>

#include "SMath.h"
#include "SModelStruct.h"
#include "SAnimator.h"

class SModel;
class SAnimationStruct;
class SImageLoader;
class SModelLoader : public SLoader
{
public:
	bool Initialize();

	SModel	LoadModelFromFile(const wchar_t* file);


private:
	void	LoadMeshes(SModel* model, SAnimationStruct* animStruct, const aiScene* pScene);
	void	LoadBones(SModel* model, SAnimationStruct* animStruct, aiMesh* pMesh, std::map<unsigned int, std::vector<SBoneWeight>>& vertexBoneWeight);

	void	LoadAnimation(SAnimationStruct* animStruct, const aiScene* pScene);
	SBone	CreateBoneTree(SAnimationStruct* animStruct, aiNode* node);
	void	CreateGlobalTransformation(SBone& child);

	SMatrix MatrixFromAI(const aiMatrix4x4& aiMatrix);
	SVector3 Vector3FromAI(const aiVector3D& aiVector3);
	SQuaternion QuatFromAI(const aiQuaternion& aiQuat);
	std::vector<SAnimTimelineVector> ATVFromAI(const aiVectorKey* pVectorKey, const unsigned int num);
	std::vector<SAnimTimelineQuat> ATQFromAI(const aiQuatKey* pQuatKey, const unsigned int num);

protected:
	SImageLoader* m_ImageLoader;
};