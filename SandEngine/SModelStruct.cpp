#include "SModelStruct.h"
#include "SAnimator.h"

namespace {
	const std::string DEFAULT_CLIP_NAME = "idle";
}

void SModel::Update(double delta)
{
	if (Animation)
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
	return Animation->HasAnimation(); 
}

std::vector<SMatrix> SModel::GetFinalTransform() const
{ 
	return Animation->GetTransform(); 
}

void SModel::AddBoneData(unsigned int vertexIndex, unsigned int boneIndex, float weight)
{
	for (unsigned int i = 0; i < BONES_PER_VERTEX; i++) {
		if (Vertices[vertexIndex].weights[i] == 0.0f) {
			Vertices[vertexIndex].boneIDs[i] = boneIndex;
			Vertices[vertexIndex].weights[i] = weight;
			return;
		}
	}
}
