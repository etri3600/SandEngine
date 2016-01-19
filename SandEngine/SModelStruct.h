#pragma once

#include "SMath.h"
#include "SUtils.h"
#include "SImage.h"

#define BONES_PER_VERTEX 4

struct SBoneWeight
{
	SBoneWeight(unsigned int boneIndex, float weight)
		:boneIndex(boneIndex), weight(weight)
	{}
	unsigned int boneIndex;
	float weight;
};

struct SBone
{
	std::string name;
	SMatrix boneOffset;
	SMatrix localTransform, globalTransform, originLocalTransform;
	SBone* parent;
	std::vector<SBone> children;
};

struct SModelVertex
{
	SVector3 position;
	SVector4 color;
	SVector3 normal;
	SVector4 tangent;
	SVector2 uv;
	unsigned int boneIDs[BONES_PER_VERTEX]{};
	float wieghts = 0.0f;
};

class SAnimationStruct;
class SAnimator;
class SModel
{
public:
	std::vector<SModelVertex> Vertices;
	std::vector<unsigned int> Indices;
	std::vector<STexture*> Textures;
	std::vector<SBone> Bones;
	SAnimationStruct* AnimationStruct;
	SAnimator* Animator;

	SVector3 Location;
	SQuaternion Rotation;
	SVector3 Scale{ 1.0f, 1.0f, 1.0f };

	void Update(double delta);
	
	static constexpr std::size_t VertexBaseSize() {
		return sizeof(SModelVertex);
	}

	auto VertexSize() const {
		return VertexBaseSize() * Vertices.size();
	}

	auto VertexCount() const {
		return Vertices.size();
	}

	void* VertexSerialize() {
		return static_cast<void*>(Vertices.data());
	}

	static constexpr std::size_t IndexBaseSize() {
		return sizeof(unsigned int);
	}

	auto IndexSize() const {
		return IndexBaseSize() * Indices.size();
	}

	auto IndexCount() const {
		return Indices.size();
	}

	void* IndexSerialize() {
		return static_cast<void*>(Indices.data());
	}

	auto GetTextures() const {
		return Textures;
	}

	STexture* GetTexture(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex] : nullptr;
	}

	unsigned int TextureSize(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetCurrentMipTexture().Size : 0U;
	}

	unsigned int TextureWidth(unsigned int nTextureIndex) const {
		unsigned int Width = 0;
		if (Textures.size() > nTextureIndex)
		{
			switch (Textures[nTextureIndex]->eTextureLayout)
			{
			case STexture::TextureLayout::TL_TEX_1D:
				Width = Textures[nTextureIndex]->GetCurrentMipTexture().Size;
				break;
			case STexture::TextureLayout::TL_TEX_2D:
			case STexture::TextureLayout::TL_TEX_3D:
				Width = Textures[nTextureIndex]->GetCurrentMipTexture().Width;
				break;
			}
		}
		return Width;
	}

	unsigned int TextureHeight(unsigned int nTextureIndex) const {
		unsigned int Height = 0;
		if (Textures.size() > nTextureIndex)
		{
			switch (Textures[nTextureIndex]->eTextureLayout)
			{
			case STexture::TextureLayout::TL_TEX_1D:
				Height = Textures[nTextureIndex]->GetCurrentMipTexture().Size;
				break;
			case STexture::TextureLayout::TL_TEX_2D:
			case STexture::TextureLayout::TL_TEX_3D:
				Height = Textures[nTextureIndex]->GetCurrentMipTexture().Height;
				break;
			}
		}
		return Height;
	}

	void* TextureSerialize(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetCurrentMipTexture().pTexData : nullptr;
	}

	unsigned int TextureFormat(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetTextureFormat() : 0;
	}

	void Release()
	{
		Vertices.clear();
		Indices.clear();
		for (auto it = Textures.begin(); it != Textures.end(); ++it)
		{
			delete[] *it;
			(*it) = nullptr;
		}
		Textures.clear();
		if (AnimationStruct)
		{
			delete AnimationStruct;
			AnimationStruct = nullptr;
		}
		if (Animator)
		{
			delete Animator;
			Animator = nullptr;
		}
	}
};

class STriangle : public SModel
{
public:
	STriangle::STriangle()
	{
		SModelVertex point1, point2, point3;
		point1.position = SVector3(0.0f, 1.0f, 0.0f);
		point1.color = { 1.0f, 0.0f, 0.0f };
		point1.normal = SVector3(0.0f, 0.0f, 1.0f);
		point1.uv = SVector2(0.0f, 0.0f);

		point2.position = SVector3(-1.0f, 0.0f, 0.0f);
		point2.color = { 0.0f, 1.0f, 0.0f };
		point2.normal = SVector3(0.0f, 0.0f, 1.0f);
		point2.uv = SVector2(0.0f, 1.0f);

		point3.position = SVector3(1.0f, 0.0f, 0.0f);
		point3.color = { 0.0f, 0.0f, 1.0f };
		point3.normal = SVector3(0.0f, 0.0f, 1.0f);
		point3.uv = SVector2(1.0f, 0.0f);
		
		Vertices.push_back(point1);
		Vertices.push_back(point2);
		Vertices.push_back(point3);

		Indices.push_back(0);
		Indices.push_back(1);
		Indices.push_back(2);
	}
};

class SCube : public SModel
{
public:
	SCube::SCube()
	{
		SModelVertex point1, point2, point3, point4, point5, point6, point7, point8;
		point1.position = { -1.0f, 1.0f, 1.0f };
		point1.color = {1.0f, 1.0f, 1.0f};
		point1.normal = { 0.0f, 0.0f, 1.0f };
		point1.uv = { 0.0f, 0.0f };

		point2.position = { -1.0f, -1.0f, 1.0f };
		point2.color = { 0.0f, 1.0f, 1.0f };
		point2.normal = { 0.0f, 0.0f, 1.0f };
		point2.uv = { 0.0f, 1.0f };

		point3.position = SVector3(1.0f, 1.0f, 1.0f);
		point3.color = { 1.0f, 0.0f, 1.0f };
		point3.normal = SVector3(0.0f, 0.0f, 1.0f);
		point3.uv = SVector2(1.0f, 0.0f);

		point4.position = SVector3(1.0f, -1.0f, 1.0f);
		point4.color = { 1.0f, 1.0f, 0.0f };
		point4.normal = SVector3(0.0f, 0.0f, 1.0f);
		point4.uv = SVector2(0.0f, 0.0f);

		point5.position = SVector3(1.0f, 1.0f, -1.0f);
		point5.color = { 0.0f, 0.0f, 1.0f };
		point5.normal = SVector3(0.0f, 0.0f, -1.0f);
		point5.uv = SVector2(0.0f, 1.0f);

		point6.position = SVector3(1.0f, -1.0f, -1.0f);
		point6.color = { 1.0f, 0.0f, 0.0f };
		point6.normal = SVector3(0.0f, 0.0f, -1.0f);
		point6.uv = SVector2(1.0f, 0.0f);

		point7.position = SVector3(-1.0f, 1.0f, -1.0f);
		point7.color = { 0.0f, 1.0f, 0.0f };
		point7.normal = SVector3(0.0f, 0.0f, -1.0f);
		point7.uv = SVector2(0.0f, 0.0f);

		point8.position = SVector3(-1.0f, -1.0f, -1.0f);
		point8.color = { 0.0f, 0.0f, 0.0f };
		point8.normal = SVector3(0.0f, 0.0f, -1.0f);
		point8.uv = SVector2(0.0f, 1.0f);

		Vertices.push_back(point1);
		Vertices.push_back(point2);
		Vertices.push_back(point3);
		Vertices.push_back(point4);
		Vertices.push_back(point5);
		Vertices.push_back(point6);
		Vertices.push_back(point7);
		Vertices.push_back(point8);

		// front
		Indices.push_back(0);
		Indices.push_back(1);
		Indices.push_back(2);

		Indices.push_back(2);
		Indices.push_back(1);
		Indices.push_back(3);

		// back
		Indices.push_back(4);
		Indices.push_back(5);
		Indices.push_back(6);

		Indices.push_back(6);
		Indices.push_back(5);
		Indices.push_back(7);

		// top
		Indices.push_back(6);
		Indices.push_back(0);
		Indices.push_back(4);

		Indices.push_back(4);
		Indices.push_back(0);
		Indices.push_back(2);

		// bottom
		Indices.push_back(1);
		Indices.push_back(7);
		Indices.push_back(3);

		Indices.push_back(3);
		Indices.push_back(7);
		Indices.push_back(5);

		// right
		Indices.push_back(2);
		Indices.push_back(3);
		Indices.push_back(4);

		Indices.push_back(4);
		Indices.push_back(3);
		Indices.push_back(5);

		// left
		Indices.push_back(6);
		Indices.push_back(7);
		Indices.push_back(0);

		Indices.push_back(0);
		Indices.push_back(7);
		Indices.push_back(1);
	}
};