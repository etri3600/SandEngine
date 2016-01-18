#include "SModelLoader.h"

#include <set>

#include "SModelStruct.h"
#include "SAnimator.h"
#include "SImageLoader.h"

namespace {
	const std::wstring WDirPath = LR"(..\Model\)";
	const std::string DirPath = R"(..\Model\)";
}

bool SModelLoader::Initialize()
{
	m_ImageLoader = new SImageLoader();
	m_ImageLoader->Initialize();
	return true;
}

SModel SModelLoader::LoadModelFromFile(const wchar_t* file)
{
	Assimp::Importer importer;
	std::wstring wfullPath = WDirPath + file;
	size_t lastPos = wfullPath.find_last_of(L"\\");
	std::wstring directory;
	if (std::wstring::npos != lastPos)
	{
		directory = wfullPath.substr(0, lastPos);
	}

	const aiScene* pScene = importer.ReadFile(Sand::WStringToString(&wfullPath),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	SModel model;
	SAnimator animator;

	if (pScene == nullptr)
	{
		perror("No File");
		return std::move(model);
	}

	LoadAnimation(&animator, pScene);
	LoadMeshes(&model, &animator, pScene);

	model.Textures.reserve(pScene->mNumMaterials);
	for (unsigned int i = 0;i < pScene->mNumMaterials; ++i)
	{
		const auto pMaterial = pScene->mMaterials[i];
		aiString texturePath;
		if (pMaterial)
		{
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
			{
				STexture* texture = new STexture();
				std::string texturePath(texturePath.C_Str());
				std::wstring fullPath = directory + Sand::StringToWString(&texturePath);
				m_ImageLoader->LoadTextureFromFile(fullPath.c_str(), texture);
				model.Textures.push_back(texture);
			}
		}
	}

	for (unsigned int i = 0;i < pScene->mNumAnimations;++i)
	{
		pScene->mAnimations[i];
	}

	return std::move(model);
}

void SModelLoader::LoadMeshes(SModel* model, SAnimator* animator, const aiScene * pScene)
{
	SModelVertex vertex;

	std::map<unsigned int, std::vector<SBoneWeight>> VertexToBoneWeight;

	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		const auto pMesh = pScene->mMeshes[i];
		if (pMesh)
		{
			for (unsigned int j = 0;j < pMesh->mNumFaces; ++j)
			{
				const auto face = pMesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					const auto index = face.mIndices[k];

					aiVector3D pos = pMesh->mVertices[index];
					aiVector3D uv = pMesh->HasTextureCoords(index) ? pMesh->mTextureCoords[0][index] : aiVector3D(0.0f, 0.0f, 0.0f);
					aiVector3D normal = pMesh->HasNormals() ? pMesh->mNormals[index] : aiVector3D(1.f, 1.f, 1.f);
					aiColor4D color = pMesh->HasVertexColors(index) ? *pMesh->mColors[index] : aiColor4D(0, 1.0f, 0, 1.0f);
					vertex.position = SVector3(pos.x, pos.y, pos.z);
					vertex.color = SVector4(color.g, color.b, color.r, color.a);
					vertex.normal = SVector3(normal.x, normal.y, normal.z);
					vertex.uv = SVector2(uv.x, uv.y);
					model->Vertices.push_back(vertex);
					model->Indices.push_back(index);
				}
			}

			LoadBones(model, animator, pMesh, VertexToBoneWeight);
		}
	}
}

void SModelLoader::LoadBones(SModel* model, SAnimator* animator, aiMesh* pMesh, std::map<unsigned int, std::vector<SBoneWeight>>& vertexBoneWeight)
{
	for (unsigned int i = 0;i < pMesh->mNumBones; ++i)
	{
		unsigned int boneIndex = animator->m_BoneIndex[pMesh->mBones[i]->mName.data];

		for (unsigned int j = 0;j < pMesh->mBones[i]->mNumWeights; ++j)
		{
			auto VertexId = pMesh->mBones[i]->mWeights[j].mVertexId;
			vertexBoneWeight[VertexId].push_back({ boneIndex, pMesh->mBones[i]->mWeights[j].mWeight });
		}
	}
}

void SModelLoader::LoadAnimation(SAnimator * animator, const aiScene * pScene)
{
	SBone rootBone = CreateBoneTree(animator, pScene->mRootNode);
	for (unsigned int i = 0;i < pScene->mNumMeshes; ++i)
	{
		std::set<std::string> boneNames;
		for (unsigned int j = 0;j < pScene->mMeshes[i]->mNumBones; ++j)
		{
			boneNames.insert(pScene->mMeshes[i]->mBones[j]->mName.data);
			if (animator->m_BoneNameMap.find(pScene->mMeshes[i]->mBones[j]->mName.data) != animator->m_BoneNameMap.end())
			{
				bool bSkip = false;
				for (auto bone : animator->m_Bones)
				{
					if (bone.name == pScene->mMeshes[i]->mBones[j]->mName.data)
					{
						bSkip = true;
						break;
					}
				}
				if (bSkip) continue;

				SBone newBone;
				newBone.boneOffset = SMath::Transpose(MatrixFromAI(pScene->mMeshes[i]->mBones[j]->mOffsetMatrix));
				animator->m_Bones.push_back(newBone);
				animator->m_BoneIndex[newBone.name] = animator->m_Bones.size() - 1;
			}
		}
		for (auto boneName : animator->m_BoneNameMap)
		{
			if (boneNames.find(boneName.first) == boneNames.end() && boneName.first.compare(0, 4, "Bone") == 0)
			{
				animator->m_BoneNameMap[boneName.first].boneOffset = animator->m_BoneNameMap[boneName.first].parent->boneOffset;
				animator->m_Bones.push_back(animator->m_BoneNameMap[boneName.first]);
				animator->m_BoneIndex[boneName.first] = animator->m_Bones.size() - 1;
			}
		}
	}

	for (unsigned int i = 0;i < pScene->mNumAnimations; ++i)
	{
		SAnimInfo info;
		info.name = pScene->mAnimations[i]->mName.data;
		info.tickPerSeconds = pScene->mAnimations[i]->mTicksPerSecond;
		info.duration = pScene->mAnimations[i]->mDuration;
		for (unsigned int j = 0;j < pScene->mAnimations[i]->mNumChannels; ++j)
		{
			SAnimChannel channel;
			channel.name = pScene->mAnimations[i]->mChannels[j]->mNodeName.data;
			channel.positions = ATVFromAI(pScene->mAnimations[i]->mChannels[j]->mPositionKeys, pScene->mAnimations[i]->mChannels[j]->mNumPositionKeys);
			channel.rotations = ATQFromAI(pScene->mAnimations[i]->mChannels[j]->mRotationKeys, pScene->mAnimations[i]->mChannels[j]->mNumRotationKeys);
			channel.scales = ATVFromAI(pScene->mAnimations[i]->mChannels[j]->mScalingKeys, pScene->mAnimations[i]->mChannels[j]->mNumScalingKeys);
			info.channels.push_back(channel);
		}
		info.lastPositions.assign(pScene->mAnimations[i]->mNumChannels, std::make_tuple(0, 0, 0));
		animator->m_Animations.push_back(info);
	}

	for (auto it = animator->m_Animations.begin(); it != animator->m_Animations.end(); ++it)
	{
		animator->m_AnimNameIdMap[it->name] = std::distance(animator->m_Animations.begin(), it);
	}
	animator->m_CurrentAnimIndex = 0;
}

SBone SModelLoader::CreateBoneTree(SAnimator* animator, aiNode* node)
{
	if (animator && node)
	{
		SBone bone;
		bone.name = node->mName.data;
		bone.parent = nullptr;
		animator->m_BoneNameMap[bone.name] = bone;
		SMatrix localMat = MatrixFromAI(node->mTransformation);
		auto inverLocalMat = SMath::Transpose(localMat);
		bone.localTransform = localMat;
		bone.originLocalTransform = bone.localTransform;
		CreateGlobalTransformation(bone);

		for (unsigned int i = 0;i < node->mNumChildren; ++i)
		{
			SBone child = CreateBoneTree(animator, node->mChildren[i]);
			bone.children.push_back(child);
		}

		return bone;
	}

	return SBone();
}

void SModelLoader::CreateGlobalTransformation(SBone& child)
{
	child.globalTransform = child.localTransform;
	SBone* parent = child.parent;
	while (parent != nullptr)
	{
		child.localTransform *= parent->localTransform;
		parent = parent->parent;
	}
}

SMatrix SModelLoader::MatrixFromAI(const aiMatrix4x4& aiMatrix)
{
	SMatrix mat;

	mat.m[0][0] = aiMatrix[0][0]; mat.m[0][1] = aiMatrix[1][0]; mat.m[0][2] = aiMatrix[2][0]; mat.m[0][3] = aiMatrix[3][0];
	mat.m[1][0] = aiMatrix[0][1]; mat.m[1][1] = aiMatrix[1][1]; mat.m[1][2] = aiMatrix[2][1]; mat.m[1][3] = aiMatrix[3][1];
	mat.m[2][0] = aiMatrix[0][2]; mat.m[2][1] = aiMatrix[1][2]; mat.m[2][2] = aiMatrix[2][2]; mat.m[2][3] = aiMatrix[3][2];
	mat.m[3][0] = aiMatrix[0][3]; mat.m[3][1] = aiMatrix[1][3]; mat.m[3][2] = aiMatrix[2][3]; mat.m[3][3] = aiMatrix[3][3];

	return mat;
}

SVector3 SModelLoader::Vector3FromAI(const aiVector3D & aiVector3)
{
	return SVector3(aiVector3.x, aiVector3.y, aiVector3.z);
}

SQuaternion SModelLoader::QuatFromAI(const aiQuaternion & aiQuat)
{
	return SQuaternion(aiQuat.x, aiQuat.y, aiQuat.z, aiQuat.w);
}

std::vector<SAnimTimelineVector> SModelLoader::ATVFromAI(const aiVectorKey * pVectorKey, const unsigned int num)
{
	std::vector<SAnimTimelineVector> ret;

	for (unsigned int i = 0;i < num; ++i)
	{
		SAnimTimelineVector atv;
		atv.time = pVectorKey[i].mTime;
		atv.vec = Vector3FromAI(pVectorKey[i].mValue);
		ret.push_back(atv);
	}

	return ret;
}

std::vector<SAnimTimelineQuat> SModelLoader::ATQFromAI(const aiQuatKey * pQuatKey, const unsigned int num)
{
	std::vector<SAnimTimelineQuat> ret;

	for (unsigned int i = 0;i < num; ++i)
	{
		SAnimTimelineQuat atv;
		atv.time = pQuatKey[i].mTime;
		atv.quat = QuatFromAI(pQuatKey[i].mValue);
		ret.push_back(atv);
	}

	return ret;
}

