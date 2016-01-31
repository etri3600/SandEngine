#include "SAnimation.h"
#include "SBone.h"

#include <cmath>
#include <Windows.h>

bool SAnimation::SetAnimation(const std::string clipName)
{
	if (m_AnimNameMap.find(clipName) != m_AnimNameMap.end()) {
		auto oldIndex = m_CurrentAnimIndex;
		m_CurrentAnimIndex = m_AnimNameMap[clipName];
		return oldIndex != m_CurrentAnimIndex;
	}
	return false;
}

void SAnimation::Update(double delta)
{
	m_ElapsedTime += delta;
	SMatrix Identity = SMatrix::Identity;

	auto animInfo = GetCurrentAnimInfo();
	
	double TicksPerSecond = (animInfo.tickPerSeconds != 0 ? animInfo.tickPerSeconds : 30.0);
	double TimeInTicks = m_ElapsedTime * TicksPerSecond;
	double AnimationTime = fmod(TimeInTicks, animInfo.duration);
	
	ReadNodeHeirarchy(AnimationTime, m_Skeleton, Identity);

	Transforms.resize(m_Bones.size());

	for (unsigned int i = 0; i < m_Bones.size(); ++i) {
		Transforms[i] = m_Bones[i]->globalTransform;
	}
}

void SAnimation::ReadNodeHeirarchy(double AnimationTime, const SBoneNode* pNode, const SMatrix & ParentTransform)
{
	auto animInfo = GetCurrentAnimInfo();
	auto channel = FindNodeChannel(&animInfo, pNode->name);
	SMatrix NodeTransformation(pNode->localTransformation);

	if (channel.boneName.size() > 0)
	{
		SVector3 scale;
		CalcInterpolatedScaling(scale, AnimationTime, channel);
		SMatrix scaleMatrix;
		scaleMatrix.Scale(scale);

		SQuaternion rotation;
		CalcInterpolatedRotation(rotation, AnimationTime, channel);
		SMatrix rotateMatrix;
		rotateMatrix.Rotate(rotation);

		SVector3 translate;
		CalcInterpolatedPosition(translate, AnimationTime, channel);
		SMatrix translateMatrix;
		translateMatrix.Translate(translate);

		NodeTransformation = translateMatrix * rotateMatrix * scaleMatrix;
	}

	SMatrix GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneNameMap.find(pNode->name) != m_BoneNameMap.end())
	{
		unsigned int BoneIndex = m_BoneNameMap[pNode->name];
		m_Bones[BoneIndex]->globalTransform = GlobalInverseTransformation * GlobalTransformation * m_Bones[BoneIndex]->boneOffset;
	}

	for (unsigned int i = 0; i < pNode->children.size(); ++i) {
		ReadNodeHeirarchy(AnimationTime, pNode->children[i], GlobalTransformation);
	}
}

SAnimChannel SAnimation::FindNodeChannel(const SAnimInfo* pAnimInfo, const std::string& boneName)
{
	for (unsigned int i = 0; i < pAnimInfo->channels.size(); ++i) {
		const SAnimChannel channel = pAnimInfo->channels[i];
		if (channel.boneName == boneName)
			return channel;
	}

	return SAnimChannel();
}

void SAnimation::CalcInterpolatedScaling(SVector3 & Out, double AnimationTime, const SAnimChannel& channel)
{
	if (channel.scales.size() == 1)
	{
		Out = channel.scales[0].vec;
		return;
	}

	unsigned int currentIndex = 0;
	for (unsigned int i = 0; i < channel.scales.size() - 1; ++i) {
		if (AnimationTime < channel.scales[i + 1].time) {
			currentIndex = i;
			break;
		}
	}

	unsigned int nextIndex = (currentIndex + 1);
	const double DeltaTime = channel.scales[nextIndex].time - channel.scales[currentIndex].time;
	double Factor = (AnimationTime - channel.scales[currentIndex].time) / DeltaTime;
	Factor = SMath::Clamp(Factor, 0.0, 1.0);
	const SVector3& Start = channel.positions[currentIndex].vec;
	const SVector3& End = channel.positions[nextIndex].vec;
	SVector3 Delta = End - Start;
	Out = Start + Delta * Factor;
}

void SAnimation::CalcInterpolatedRotation(SQuaternion & Out, double AnimationTime, const SAnimChannel& channel)
{
	if (channel.rotations.size() == 1)
	{
		Out = channel.rotations[0].quat;
		return;
	}

	unsigned int currentIndex = 0;
	for (unsigned int i = 0; i < channel.rotations.size() - 1; ++i) {
		if (AnimationTime < channel.rotations[i + 1].time) {
			currentIndex = i;
			break;
		}
	}

	unsigned int nextIndex = (currentIndex + 1);
	const double DeltaTime = channel.rotations[nextIndex].time - channel.rotations[currentIndex].time;
	float Factor = static_cast<float>((AnimationTime - channel.rotations[currentIndex].time) / DeltaTime);
	Factor = SMath::Clamp(Factor, 0.0f, 1.0f);
	const SQuaternion& Start = channel.rotations[currentIndex].quat;
	const SQuaternion& End = channel.rotations[nextIndex].quat;
	Out = SMath::Slerp(Start, End, Factor);
	Out.Normalize();
}

void SAnimation::CalcInterpolatedPosition(SVector3 & Out, double AnimationTime, const SAnimChannel& channel)
{
	if (channel.positions.size() == 1)
	{
		Out = channel.positions[0].vec;
		return;
	}

	unsigned int PositionIndex = 0;
	for (unsigned int i = 0; i < channel.positions.size() - 1; ++i) {
		if (AnimationTime < channel.positions[i + 1].time) {
			PositionIndex = i;
			break;
		}
	}

	unsigned int NextPositionIndex = (PositionIndex + 1);
	const double DeltaTime = channel.positions[NextPositionIndex].time - channel.positions[PositionIndex].time;
	double Factor = (AnimationTime - channel.positions[PositionIndex].time) / DeltaTime;
	Factor = SMath::Clamp(Factor, 0.0, 1.0);
	const SVector3& Start = channel.positions[PositionIndex].vec;
	const SVector3& End = channel.positions[NextPositionIndex].vec;
	SVector3 Delta = End - Start;
	Out = Start + Delta * Factor;
}

void SAnimation::CalculateBoneNodeToWorldTransform(SBoneNode* child)
{
	child->globalTransformation = child->localTransformation;
	SBoneNode* parent = child->parent;
	while (parent != nullptr)
	{
		child->localTransformation *= parent->localTransformation;
		parent = parent->parent;
	}
}