#include "SModelLoader.h"

#include <set>

#include "SModelStruct.h"
#include "SAnimation.h"
#include "SImageLoader.h"
#include "SBone.h"

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
		directory = wfullPath.substr(0, lastPos + 1);
	}

	const aiScene* pScene = importer.ReadFile(Sand::WStringToString(&wfullPath),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		//aiProcess_JoinIdenticalVertices | 
		aiProcess_GenSmoothNormals);

	SModel model;
	if (pScene == nullptr)
	{
		assert(pScene);
		return std::move(model);
	}

	model.Animation = new SAnimation();

	LoadMeshes(&model, pScene);
	LoadTextures(&model, directory, pScene);
	LoadAnimation(model.Animation, pScene);

	if (model.Animation->HasAnimation())
	{
		model.SetClipName(model.Animation->AnimName());
		for (auto clip : model.GetClips())
		{
			model.AddClip(clip);
		}
	}

	return std::move(model);
}

void SModelLoader::LoadMeshes(SModel* model, const aiScene* pScene)
{
	model->MeshInfoes.resize(pScene->mNumMeshes);
	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	for (unsigned int i = 0; i < model->MeshInfoes.size(); ++i) {
		model->MeshInfoes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		model->MeshInfoes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		model->MeshInfoes[i].BaseVertex = NumVertices;
		model->MeshInfoes[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += model->MeshInfoes[i].NumIndices;
	}

	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		const auto pMesh = pScene->mMeshes[i];
		if (pMesh)
		{
			for (unsigned int j = 0;j < pMesh->mNumVertices; ++j)
			{
				SSkinnedModelVertex vertex;
				vertex.position = Vector3FromAI(pMesh->mVertices[j]);
				vertex.normal = pMesh->HasNormals() ? Vector3FromAI(pMesh->mNormals[j]) : SVector3(1.f, 1.f, 1.f);
				if (pMesh->HasTangentsAndBitangents())
					vertex.tangent = Vector3FromAI(pMesh->mTangents[j]);
				SVector3 uv = pMesh->HasTextureCoords(0) ? Vector3FromAI(pMesh->mTextureCoords[0][j]) : SVector3(0.0f, 0.0f, 0.0f);
				vertex.uv = { uv.x, uv.y };
				vertex.color = pMesh->HasVertexColors(0) ? Vector4FromAI(pMesh->mColors[0][j]) : SVector4(0, 1.0f, 0, 1.0f);
				model->Vertices.push_back(vertex);
			}
			for (unsigned int j = 0;j < pMesh->mNumFaces; ++j)
			{
				const auto face = pMesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					const auto index = face.mIndices[k];
					model->Indices.push_back(index);
				}
			}
			
			LoadBones(model, pMesh, i);
		}
	}
}
void SModelLoader::LoadBones(SModel* model, aiMesh* pMesh, unsigned int meshIndex)
{
	for (unsigned int i = 0;i < pMesh->mNumBones; ++i)
	{
		unsigned int BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);
		if (model->Animation->m_BoneNameMap.find(BoneName) == model->Animation->m_BoneNameMap.end()) {
			SBone* bone = new SBone();
			BoneIndex = model->Animation->m_NumBones++;
			model->Animation->m_Bones.push_back(bone);
			bone->boneOffset = MatrixFromAI(pMesh->mBones[i]->mOffsetMatrix);
			model->Animation->m_BoneNameMap[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = model->Animation->m_BoneNameMap[BoneName];
		}
		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; ++j) {
			unsigned int VertexID = model->MeshInfoes[meshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			model->AddBoneData(VertexID, BoneIndex, Weight);
		}
	}
	if(pMesh->mNumBones == 0)
	{
		model->SetDefaultBoneWeights();
	}
}

void SModelLoader::LoadTextures(SModel * model, std::wstring & directory, const aiScene * pScene)
{
	model->Textures.reserve(pScene->mNumMaterials);
	for (unsigned int i = 0;i < pScene->mNumMaterials; ++i)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		aiString texturePath;
		if (pMaterial)
		{
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
			{
				STexture* texture = new STexture();
				std::string texturePath(texturePath.data);
				std::wstring fullPath = directory + Sand::StringToWString(&texturePath);
				m_ImageLoader->LoadTextureFromFile(fullPath.c_str(), texture);
				model->Textures.push_back(texture);
			}
		}
	}
}

void SModelLoader::LoadAnimation(SAnimation * animation, const aiScene * pScene)
{
	animation->m_Skeleton = CreateBoneTree(animation, nullptr, pScene->mRootNode);
	animation->GlobalInverseTransformation = MatrixFromAI(pScene->mRootNode->mTransformation).Inverse();
	for (unsigned int i = 0; i < pScene->mNumAnimations; ++i)
	{
		SAnimInfo animInfo;
		animInfo.duration = pScene->mAnimations[i]->mDuration;
		animInfo.tickPerSeconds = pScene->mAnimations[i]->mTicksPerSecond;
		animInfo.name = pScene->mAnimations[i]->mName.data;
		for (unsigned int j = 0;j < pScene->mAnimations[i]->mNumChannels;++j)
		{
			SAnimChannel channel;
			channel.boneName = pScene->mAnimations[i]->mChannels[j]->mNodeName.data;
			channel.scales = ATVFromAI(pScene->mAnimations[i]->mChannels[j]->mScalingKeys, pScene->mAnimations[i]->mChannels[j]->mNumScalingKeys);
			channel.rotations = ATQFromAI(pScene->mAnimations[i]->mChannels[j]->mRotationKeys, pScene->mAnimations[i]->mChannels[j]->mNumRotationKeys);
			channel.positions = ATVFromAI(pScene->mAnimations[i]->mChannels[j]->mPositionKeys, pScene->mAnimations[i]->mChannels[j]->mNumPositionKeys);
			animInfo.channels.push_back(channel);
		}
		animation->m_Animations.push_back(animInfo);
		animation->m_AnimNameMap.insert(std::make_pair(animInfo.name, animation->m_Animations.size() - 1));
	}
}

SBoneNode* SModelLoader::CreateBoneTree(SAnimation* animation, SBoneNode* pBoneNode, aiNode* node)
{
	if (animation && node)
	{
		SBoneNode* boneNode = new SBoneNode();
		boneNode->name = node->mName.data;
		boneNode->parent = pBoneNode;
		boneNode->localTransformation = MatrixFromAI(node->mTransformation);
		//animation->CalculateBoneNodeToWorldTransform(boneNode);
		animation->m_BoneNodeNameMap[boneNode->name] = boneNode;

		for (unsigned int i = 0;i < node->mNumChildren; ++i)
		{
			SBoneNode* child = CreateBoneTree(animation, boneNode, node->mChildren[i]);
			boneNode->children.push_back(child);
		}


		return boneNode;
	}

	return nullptr;
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

SVector4 SModelLoader::Vector4FromAI(const aiColor4D & aiColor4)
{
	return SVector4(aiColor4.r, aiColor4.g, aiColor4.b, aiColor4.a);
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

