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
	SModelVertex vertex;

	if (pScene == nullptr)
	{
		perror("No File");
		return std::move(model);
	}

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
					model.Vertices.push_back(vertex);
					model.Indices.push_back(index);
				}
			}

			for (unsigned int j = 0;j < pMesh->mNumBones; ++j)
			{
				std::string name(pMesh->mBones[j]->mName.data);
				for (unsigned int k = 0;k < pMesh->mBones[j]->mNumWeights; ++k)
				{
					SBone bone;
					bone.BoneWeightMap.insert(std::pair<unsigned int,float>(pMesh->mBones[j]->mWeights[i].mVertexId, pMesh->mBones[j]->mWeights[i].mWeight));
					model.Bones.push_back(bone);
				}
			}
		}
	}

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

	LoadModelFromNode(&model, pScene->mRootNode);

	return std::move(model);
}

void SModelLoader::LoadModelFromNode(SModel* model, aiNode* node)
{
	if (model && node)
	{

	}
}

