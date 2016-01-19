#include "SAnimator.h"

void SAnimationStruct::SetAnimation(const std::string clipName)
{

}

namespace {
	const std::string DEFAULT_CLIP_NAME = "still";
}

void SAnimator::Update(double delta)
{
	m_timePoint += delta;
	if (m_timePoint > m_model.AnimationStruct->AnimDuration())
	{
		if (m_clipQueue.size() > 0)
		{
			SetClipName(m_clipQueue.front());
			if (m_bLoopClips)
				m_clipQueue.push(m_clipName);
		}
		else
		{
			m_clipName = DEFAULT_CLIP_NAME;
		}
	}
}

void SAnimator::SetClipName(const std::string clipName)
{
	bool bFindClip = false;
	for (auto it = m_model.AnimationStruct->m_Animations.begin(); it != m_model.AnimationStruct->m_Animations.end(); ++it)
	{
		if (it->GetName() == clipName)
		{
			bFindClip;
			break;
		}
	}

	m_clipName = bFindClip ? clipName : DEFAULT_CLIP_NAME;
	m_model.AnimationStruct->SetAnimation(m_clipName);
	m_timePoint = 0.0f;
}

std::vector<std::string> SAnimator::GetClips() const
{
	std::vector<std::string> ret;
	for (auto it = m_model.AnimationStruct->m_Animations.begin(); it != m_model.AnimationStruct->m_Animations.end(); ++it)
		ret.push_back(it->GetName());

	return ret;
}
