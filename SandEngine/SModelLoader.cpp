#include "SModelLoader.h"

#include "SModelStruct.h"

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

	if (pScene == nullptr)
	{
		perror("No File");
		return std::move(model);
	}

	LoadMeshes(&model, pScene);

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

	//LoadModelFromNode(&model, pScene->mRootNode);

	return std::move(model);
}

void SModelLoader::LoadModelFromNode(SModel* model, aiNode* node)
{
	if (model && node)
	{

	}
}

void SModelLoader::LoadMeshes(SModel* model, const aiScene * pScene)
{
	SModelVertex vertex;

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

			LoadBones(model, pMesh);
		}
	}
}

void SModelLoader::LoadBones(SModel* model, aiMesh* pMesh)
{
	for (unsigned int i = 0;i < pMesh->mNumBones; ++i)
	{
		unsigned int boneIndex = 0;
		std::string name(pMesh->mBones[i]->mName.data);
		if (model->BoneMap.find(name) == model->BoneMap.end())
		{
			boneIndex = model->BoneNums++;
			SBone bone;
			model->Bones.push_back(bone);
			model->Bones[boneIndex].boneMatrix = MatrixFromAI(pMesh->mBones[i]->mOffsetMatrix);
			model->BoneMap[name] = boneIndex;
		}
		else
			boneIndex = model->BoneMap[name];

		for (unsigned int j = 0;j < pMesh->mBones[i]->mNumWeights; ++j)
		{
			auto IdWeight = std::pair<unsigned int, float>(pMesh->mBones[i]->mWeights[j].mVertexId, pMesh->mBones[i]->mWeights[j].mWeight);
			model->Bones[boneIndex].BoneWeightMap.insert(IdWeight);
		}
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

