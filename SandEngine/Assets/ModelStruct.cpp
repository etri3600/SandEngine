#include "ModelStruct.h"
#include "Animation.h"

namespace {
	const std::string DEFAULT_CLIP_NAME = "idle";
}

SModel::~SModel()
{
}

void SModel::Update(double delta)
{
	if (Animation && Animation->HasAnimation())
	{
		Animation->Update(delta);
	}
}

void SModel::SetClipName(const std::string clipName)
{
	bool bFindClip = false;
	for (auto it = Animation->m_Animations.begin(); it != Animation->m_Animations.end(); ++it)
	{
		if (it->GetName() == clipName)
		{
			bFindClip = true;
			break;
		}
	}

	m_clipName = bFindClip ? clipName : DEFAULT_CLIP_NAME;
	Animation->SetAnimation(m_clipName);
	m_timePoint = 0.0f;
}

std::vector<std::string> SModel::GetClips() const
{
	std::vector<std::string> ret;
	for (auto it = Animation->m_Animations.begin(); it != Animation->m_Animations.end(); ++it)
		ret.push_back(it->GetName());

	return ret;
}

bool SModel::HasAnimation() const 
{ 
	return Animation ? Animation->HasAnimation() : false;
}

std::vector<SMatrix> SModel::GetFinalTransform() const
{ 
	return Animation->GetTransform(); 
}

bool SModel::AddBoneData(unsigned int vertexIndex, unsigned int boneIndex, float weight)
{
	float minWeight = 1.0f;
	int minWeightIndex = -1;
	for (unsigned int i = 0; i < BONES_PER_VERTEX; i++) {
		if (minWeight > Vertices[vertexIndex].weights[i])
		{
			minWeight = Vertices[vertexIndex].weights[i];
			minWeightIndex = i;
		}
	}

	if (minWeightIndex >= 0 && minWeight < weight)
	{
		Vertices[vertexIndex].boneIDs[minWeightIndex] = boneIndex;
		Vertices[vertexIndex].weights[minWeightIndex] = weight;
	}

	return minWeightIndex != -1;
}

void SModel::SetDefaultBoneWeights()
{
	for (auto& vertex : Vertices)
	{
		for (auto& weight : vertex.weights)
		{
			weight = 1.0f / BONES_PER_VERTEX;
		}
	}
}

void SModel::Release()
{
	Vertices.clear();
	Indices.clear();
	for (auto it = Textures.begin(); it != Textures.end(); ++it)
	{
		delete[] * it;
		(*it) = nullptr;
	}
	Textures.clear();
	if (Animation)
	{
		delete Animation;
		Animation = nullptr;
	}
}
