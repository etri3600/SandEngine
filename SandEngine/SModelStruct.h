#pragma once

#include "SMath.h"
#include "SUtils.h"
#include "SImage.h"

#include <queue>

#define BONES_PER_VERTEX 4U

struct SModelVertex
{
	SVector3 position;
	SVector4 color;
	SVector3 normal;
	SVector4 tangent;
	SVector2 uv;
	unsigned int boneIDs[BONES_PER_VERTEX]{};
	float weights[BONES_PER_VERTEX]{};
};

struct SMeshInfo
{
	unsigned int NumIndices = 0;
	unsigned int BaseVertex = 0;
	unsigned int BaseIndex = 0;
	unsigned int MaterialIndex = -1;
	std::vector<SModelVertex> Vertices;
	std::vector<unsigned int> Indices;
};

class SAnimation;
class SModel
{
public:
	~SModel();

	std::vector<SModelVertex> Vertices;
	std::vector<unsigned int> Indices;
	SAnimation* Animation;

	std::vector<SMeshInfo> MeshInfoes;
	std::vector<STexture*> Textures;

	SVector3 Location;
	SQuaternion Rotation;
	SVector3 Scale{ 1.0f, 1.0f, 1.0f };

	double m_timePoint;
	SMatrix m_world;
	std::string m_clipName;
	std::queue<std::string> m_clipQueue;
	bool m_bLoopClips;

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

	std::vector<STexture*> GetTextures() const {
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
			Width = Textures[nTextureIndex]->GetWidth();
		return Width;
	}

	unsigned int TextureHeight(unsigned int nTextureIndex) const {
		unsigned int Height = 0;
		if (Textures.size() > nTextureIndex)
		{
			Height = Textures[nTextureIndex]->GetHeight();
		}
		return Height;
	}

	void* TextureSerialize(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetCurrentMipTexture().pTexData : nullptr;
	}

	unsigned int TextureFormat(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetTextureFormat() : 0;
	}

	unsigned int TexelSize(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetTexelSize() : 0;
	}

	std::string GetClipName() const { return m_clipName; }
	void SetClipName(const std::string clipName);
	std::vector<std::string> GetClips() const;
	void AddClip(const std::string& clip) { m_clipQueue.push(clip); }
	void ClearClip() { std::queue<std::string> empty; std::swap(m_clipQueue, empty); }

	bool HasAnimation() const;
	std::vector<SMatrix> GetFinalTransform() const;

	bool AddBoneData(unsigned int vertexIndex, unsigned int boneIndex, float weight);
	void SetDefaultBoneWeights();

	void Release();
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
		point1.weights[0] = 1.0f;

		point2.position = SVector3(-1.0f, 0.0f, 0.0f);
		point2.color = { 0.0f, 1.0f, 0.0f };
		point2.normal = SVector3(0.0f, 0.0f, 1.0f);
		point2.uv = SVector2(0.0f, 1.0f);
		point2.weights[0] = 1.0f;

		point3.position = SVector3(1.0f, 0.0f, 0.0f);
		point3.color = { 0.0f, 0.0f, 1.0f };
		point3.normal = SVector3(0.0f, 0.0f, 1.0f);
		point3.uv = SVector2(1.0f, 0.0f);
		point3.weights[0] = 1.0f;
		
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
		point1.weights[0] = 1.0f;

		point2.position = { -1.0f, -1.0f, 1.0f };
		point2.color = { 0.0f, 1.0f, 1.0f };
		point2.normal = { 0.0f, 0.0f, 1.0f };
		point2.uv = { 0.0f, 1.0f };
		point2.weights[0] = 1.0f;

		point3.position = SVector3(1.0f, 1.0f, 1.0f);
		point3.color = { 1.0f, 0.0f, 1.0f };
		point3.normal = SVector3(0.0f, 0.0f, 1.0f);
		point3.uv = SVector2(1.0f, 0.0f);
		point3.weights[0] = 1.0f;

		point4.position = SVector3(1.0f, -1.0f, 1.0f);
		point4.color = { 1.0f, 1.0f, 0.0f };
		point4.normal = SVector3(0.0f, 0.0f, 1.0f);
		point4.uv = SVector2(0.0f, 0.0f);
		point4.weights[0] = 1.0f;

		point5.position = SVector3(1.0f, 1.0f, -1.0f);
		point5.color = { 0.0f, 0.0f, 1.0f };
		point5.normal = SVector3(0.0f, 0.0f, -1.0f);
		point5.uv = SVector2(0.0f, 1.0f);
		point5.weights[0] = 1.0f;

		point6.position = SVector3(1.0f, -1.0f, -1.0f);
		point6.color = { 1.0f, 0.0f, 0.0f };
		point6.normal = SVector3(0.0f, 0.0f, -1.0f);
		point6.uv = SVector2(1.0f, 0.0f);
		point6.weights[0] = 1.0f;

		point7.position = SVector3(-1.0f, 1.0f, -1.0f);
		point7.color = { 0.0f, 1.0f, 0.0f };
		point7.normal = SVector3(0.0f, 0.0f, -1.0f);
		point7.uv = SVector2(0.0f, 0.0f);
		point7.weights[0] = 1.0f;

		point8.position = SVector3(-1.0f, -1.0f, -1.0f);
		point8.color = { 0.0f, 0.0f, 0.0f };
		point8.normal = SVector3(0.0f, 0.0f, -1.0f);
		point8.uv = SVector2(0.0f, 1.0f);
		point8.weights[0] = 1.0f;

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