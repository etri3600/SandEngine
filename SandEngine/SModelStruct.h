#pragma once

#include "SMath.h"
#include "SUtils.h"
#include "SImage.h"

struct SModelVertex
{
	SVector3 position;
	SVector4 color;
	SVector3 normal;
	SVector2 uv;
};

class SModel
{
public:
	std::vector<SModelVertex> Vertices;
	std::vector<unsigned int> Indices;
	std::vector<STexture*> Textures;

	SVector3 Location;
	SQuaternion Rotation;
	SVector3 Scale{ 1.0f, 1.0f, 1.0f };
	
	constexpr std::size_t VertexBaseSize() const {
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

	constexpr std::size_t IndexBaseSize() const {
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

	unsigned int TextureSize(unsigned int nTextureIndex) const {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->Size : 0U;
	}

	unsigned int TextureWidth(unsigned int nTextureIndex) const {
		unsigned int Width = 0;
		if (Textures.size() > nTextureIndex)
		{
			switch (Textures[nTextureIndex]->eTextureLayout)
			{
			case STexture::TextureLayout::TL_TEX_1D:
				Width = Textures[nTextureIndex]->Size;
				break;
			case STexture::TextureLayout::TL_TEX_2D:
			case STexture::TextureLayout::TL_TEX_3D:
				Width = static_cast<STexture2D*>(Textures[nTextureIndex])->Width;
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
				Height = Textures[nTextureIndex]->Size;
				break;
			case STexture::TextureLayout::TL_TEX_2D:
			case STexture::TextureLayout::TL_TEX_3D:
				Height = static_cast<STexture2D*>(Textures[nTextureIndex])->Height;
				break;
			}
		}
		return Height;
	}

	void* TextureSerialize(unsigned int nTextureIndex) {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->pTexData : nullptr;
	}

	unsigned int TextureFormat(unsigned int nTextureIndex) {
		return Textures.size() > nTextureIndex ? Textures[nTextureIndex]->GetTextureFormat() : 0;
	}

	void Release()
	{
		Vertices.empty();
		Indices.empty();
		for (auto it = Textures.begin(); it != Textures.end(); ++it)
		{
			delete[] *it;
			(*it) = nullptr;
		}
		Textures.empty();
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