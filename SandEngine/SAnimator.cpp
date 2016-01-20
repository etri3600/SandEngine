#include "SAnimator.h"

bool SAnimation::SetAnimation(const std::string clipName)
{
	if (m_AnimNameIdMap.find(clipName) != m_AnimNameIdMap.end()) {
		auto oldIndex = m_CurrentAnimIndex;
		m_CurrentAnimIndex = m_AnimNameIdMap[clipName];
		return oldIndex != m_CurrentAnimIndex;
	}
	return false;
}

void SAnimation::ExtractAnimation()
{
	for (unsigned int i = 0; i < m_Animations.size(); ++i) {
		m_AnimNameIdMap[m_Animations[i].name] = i;
	}
	m_CurrentAnimIndex = 0;
	double timeStep = 1.0 / TICKS_PER_SECOND;
	for (unsigned int i = 0; i<m_Animations.size(); ++i)
	{
		SetAnimationIndex(i);
		double delta = 0.0;
		for (double tick = 0.0; tick < m_Animations[i].duration; tick += m_Animations[i].tickPerSeconds / TICKS_PER_SECOND)
		{
			delta += timeStep;
			CalculateAnimTime(delta);
			std::vector<SMatrix> trasnforms;
			for (unsigned int j = 0;j < m_Bones.size(); ++j)
			{
				auto rotation = m_Bones[j]->boneOffset * m_Bones[j]->globalTransform;
				trasnforms.push_back(rotation);
			}
			m_Animations[i].transforms.push_back(trasnforms);
		}
	}
}

void SAnimation::CalculateAnimTime(double delta)
{
	if (0 <= m_CurrentAnimIndex && m_CurrentAnimIndex < m_Animations.size())
	{
		m_Animations[m_CurrentAnimIndex].Evaluate(delta, m_BoneNameMap);
		UpdateBoneTransform(m_Skeleton);
	}
}

void SAnimation::UpdateBoneTransform(SBone* bone)
{
	CalculateBoneToWorldTransform(bone);
	for (auto child : bone->children)
	{
		UpdateBoneTransform(child);
	}
}

void SAnimation::CalculateBoneToWorldTransform(SBone* child)
{
	child->globalTransform = child->localTransform;
	SBone* parent = child->parent;
	while (parent != nullptr)
	{
		child->localTransform *= parent->localTransform;
		parent = parent->parent;
	}
}

namespace {
	const std::string DEFAULT_CLIP_NAME = "still";
}

void SAnimator::Update(double delta)
{
	m_timePoint += delta;
	if (m_model.Animation->HasAnimation() && m_timePoint > m_model.Animation->AnimDuration())
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
	for (auto it = m_model.Animation->m_Animations.begin(); it != m_model.Animation->m_Animations.end(); ++it)
	{
		if (it->GetName() == clipName)
		{
			bFindClip = true;
			break;
		}
	}

	m_clipName = bFindClip ? clipName : DEFAULT_CLIP_NAME;
	m_model.Animation->SetAnimation(m_clipName);
	m_timePoint = 0.0f;
}

std::vector<std::string> SAnimator::GetClips() const
{
	std::vector<std::string> ret;
	for (auto it = m_model.Animation->m_Animations.begin(); it != m_model.Animation->m_Animations.end(); ++it)
		ret.push_back(it->GetName());

	return ret;
}

void SAnimInfo::Evaluate(double delta, std::map<std::string, SBone*> bones)
{
	delta *= tickPerSeconds;
	double time = 0.0;
	if (duration > 0.0f) {
		time = std::fmod(delta, duration);
	}
	for (unsigned int i = 0; i < channels.size(); ++i) {
		if (bones.find(channels[i].name) == bones.end()) {
			continue;
		}

		SVector3 Position;
		if (channels[i].positions.size() > 0) {
			unsigned int frame = (time >= lastTime) ? std::get<0>(lastPositions[i]) : 0;
			while (frame < channels[i].positions.size() - 1) {
				if (time < channels[i].positions[frame + 1].time) {
					break;
				}
				frame++;
			}
			if (frame >= channels[i].positions.size()) {
				frame = 0;
			}

			unsigned int nextFrame = (frame + 1) % channels[i].positions.size();

			auto key = channels[i].positions[frame];
			auto nextKey = channels[i].positions[nextFrame];
			double diffTime = nextKey.time - key.time;
			if (diffTime < 0.0) {
				diffTime += duration;
			}
			if (diffTime > 0.0) {
				float factor = (float)((time - key.time) / diffTime);
				Position = key.vec + (nextKey.vec - key.vec) * factor;
			}
			else {
				Position = key.vec;
			}
			std::get<0>(lastPositions[i]) = frame;

		}
		// interpolate rotation keyframes
		SQuaternion rot;
		if (channels[i].rotations.size() > 0) {
			unsigned int frame = (time >= lastTime) ? std::get<1>(lastPositions[i]) : 0;
			while (frame < channels[i].rotations.size() - 1) {
				if (time < channels[i].rotations[frame + 1].time) {
					break;
				}
				frame++;
			}
			if (frame >= channels[i].rotations.size()) {
				frame = 0;
			}
			unsigned int nextFrame = (frame + 1) % channels[i].rotations.size();

			auto key = channels[i].rotations[frame];
			auto nextKey = channels[i].rotations[nextFrame];
			key.quat.Normalize();
			nextKey.quat.Normalize();
			double diffTime = nextKey.time - key.time;
			if (diffTime < 0.0) {
				diffTime += duration;
			}
			if (diffTime > 0) {
				float factor = (float)((time - key.time) / diffTime);
				rot = SMath::Slerp(key.quat, nextKey.quat, factor);
			}
			else {
				rot = key.quat;
			}
			// TODO
			std::get<1>(lastPositions[i]) = frame;

		}
		// interpolate scale keyframes
		SVector3 scale;
		if (channels[i].scales.size() > 0) {
			unsigned int frame = (time >= lastTime) ? std::get<2>(lastPositions[i]) : 0;
			while (frame < channels[i].scales.size() - 1) {
				if (time < channels[i].scales[frame + 1].time) {
					break;
				}
				frame++;
			}
			if (frame >= channels[i].scales.size()) {
				frame = 0;
			}
			std::get<2>(lastPositions[i]) = frame;
		}

		// create the combined transformation matrix
		SMatrix mat(rot.RotationMatrix());
		mat.m[0][0] *= scale.x; mat.m[1][0] *= scale.x; mat.m[2][0] *= scale.x;
		mat.m[0][1] *= scale.y; mat.m[1][1] *= scale.y; mat.m[2][1] *= scale.y;
		mat.m[0][2] *= scale.z; mat.m[1][2] *= scale.z; mat.m[2][2] *= scale.z;
		mat.m[0][3] = Position.x; mat.m[1][3] = Position.y; mat.m[2][3] = Position.z;

		bones[channels[i].name]->localTransform = mat;
	}
	lastTime = time;
}
