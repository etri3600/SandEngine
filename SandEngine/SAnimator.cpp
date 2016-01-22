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

void SAnimation::Update(double delta)
{
	SMatrix Identity = SMatrix::Identity;

	auto animInfo = GetCurrentAnimInfo();
	
	double TicksPerSecond = (animInfo.tickPerSeconds != 0 ? animInfo.tickPerSeconds : 25.0);
	double TimeInTicks = delta * TicksPerSecond;
	double AnimationTime = fmod(TimeInTicks, animInfo.duration);

	ReadNodeHeirarchy(AnimationTime, m_Skeleton, Identity);

	Transforms.resize(m_Bones.size());

	for (unsigned int i = 0; i < m_Bones.size(); ++i) {
		Transforms[i] = m_Bones[i]->globalTransform;
	}
}

void SAnimation::ReadNodeHeirarchy(double AnimationTime, const SBone* pNode, const SMatrix & ParentTransform)
{
	auto animInfo = GetCurrentAnimInfo();
	auto channel = FindNodeChannel(&animInfo, animInfo.name);
	SMatrix NodeTransformation(pNode->localTransform);

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
	translateMatrix.Translate(scale);

	NodeTransformation = translateMatrix * rotateMatrix * scaleMatrix;
	SMatrix GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneIndex.find(pNode->name) != m_BoneIndex.end())
	{
		unsigned int BoneIndex = m_BoneIndex[pNode->name];
		m_Bones[BoneIndex]->globalTransform = GlobalTransformation * m_Bones[BoneIndex]->boneOffset;
	}

	for (unsigned int i = 0; i < pNode->children.size(); ++i) {
		ReadNodeHeirarchy(AnimationTime, pNode->children[i], GlobalTransformation);
	}
}

SAnimChannel SAnimation::FindNodeChannel(const SAnimInfo* pAnimInfo, const std::string animName)
{
	for (unsigned int i = 0; i < pAnimInfo->channels.size(); ++i) {
		const SAnimChannel channel = pAnimInfo->channels[i];
		if (channel.name == animName)
			return channel;
	}

	return SAnimChannel();
}

void SAnimation::CalcInterpolatedScaling(SVector3 & Out, double AnimationTime, const SAnimChannel& channel)
{
	if (channel.scales.size() == 1)
	{
		Out = channel.scales[0].vec;
	}

	unsigned int currentIndex = 0;
	for (unsigned int i = 0; i < channel.scales.size() - 1; ++i) {
		if (AnimationTime < channel.scales[i + 1].time) {
			currentIndex = i;
			break;
		}
	}

	unsigned int nextIndex = (currentIndex + 1);
	double DeltaTime = (float)(channel.scales[nextIndex].time - channel.scales[currentIndex].time);
	double Factor = AnimationTime - channel.scales[nextIndex].time / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
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
	}

	unsigned int currentIndex = 0;
	for (unsigned int i = 0; i < channel.rotations.size() - 1; ++i) {
		if (AnimationTime < channel.rotations[i + 1].time) {
			currentIndex = i;
			break;
		}
	}

	unsigned int nextIndex = (currentIndex + 1);
	double DeltaTime = (float)(channel.rotations[nextIndex].time - channel.rotations[currentIndex].time);
	double Factor = AnimationTime - channel.rotations[nextIndex].time / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
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
	}

	unsigned int PositionIndex = 0;
	for (unsigned int i = 0; i < channel.positions.size() - 1; ++i) {
		if (AnimationTime < channel.positions[i + 1].time) {
			PositionIndex = i;
			break;
		}
	}

	unsigned int NextPositionIndex = (PositionIndex + 1);
	double DeltaTime = (float)(channel.positions[NextPositionIndex].time - channel.positions[PositionIndex].time);
	double Factor = AnimationTime - channel.positions[NextPositionIndex].time / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const SVector3& Start = channel.positions[PositionIndex].vec;
	const SVector3& End = channel.positions[NextPositionIndex].vec;
	SVector3 Delta = End - Start;
	Out = Start + Delta * Factor;
}

//void SAnimation::ExtractAnimation()
//{
//	for (unsigned int i = 0; i < m_Animations.size(); ++i) {
//		m_AnimNameIdMap[m_Animations[i].name] = i;
//	}
//	m_CurrentAnimIndex = 0;
//	double timeStep = 1.0 / TICKS_PER_SECOND;
//	for (unsigned int i = 0; i<m_Animations.size(); ++i)
//	{
//		SetAnimationIndex(i);
//		double delta = 0.0;
//		for (double tick = 0.0; tick < m_Animations[i].duration; tick += m_Animations[i].tickPerSeconds / TICKS_PER_SECOND)
//		{
//			delta += timeStep;
//			CalculateAnimTime(delta);
//			std::vector<SMatrix> trasnforms;
//			for (unsigned int j = 0;j < m_Bones.size(); ++j)
//			{
//				auto rotation = m_Bones[j]->boneOffset * m_Bones[j]->globalTransform;
//				trasnforms.push_back(rotation);
//			}
//			m_Animations[i].transforms.push_back(trasnforms);
//		}
//	}
//}
//
//void SAnimation::CalculateAnimTime(double delta)
//{
//	if (0 <= m_CurrentAnimIndex && m_CurrentAnimIndex < m_Animations.size())
//	{
//		m_Animations[m_CurrentAnimIndex].Evaluate(delta, m_BoneNameMap);
//		UpdateBoneTransform(m_Skeleton);
//	}
//}
//
//void SAnimation::UpdateBoneTransform(SBone* bone)
//{
//	CalculateBoneToWorldTransform(bone);
//	for (auto child : bone->children)
//	{
//		UpdateBoneTransform(child);
//	}
//}
//
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
//
//void SAnimInfo::Evaluate(double delta, std::map<std::string, SBone*> bones)
//{
//	delta *= tickPerSeconds;
//	double time = 0.0;
//	if (duration > 0.0f) {
//		time = std::fmod(delta, duration);
//	}
//	for (unsigned int i = 0; i < channels.size(); ++i) {
//		if (bones.find(channels[i].name) == bones.end()) {
//			continue;
//		}
//
//		SVector3 Position;
//		if (channels[i].positions.size() > 0) {
//			unsigned int frame = (time >= lastTime) ? std::get<0>(lastPositions[i]) : 0;
//			while (frame < channels[i].positions.size() - 1) {
//				if (time < channels[i].positions[frame + 1].time) {
//					break;
//				}
//				frame++;
//			}
//			if (frame >= channels[i].positions.size()) {
//				frame = 0;
//			}
//
//			unsigned int nextFrame = (frame + 1) % channels[i].positions.size();
//
//			auto key = channels[i].positions[frame];
//			auto nextKey = channels[i].positions[nextFrame];
//			double diffTime = nextKey.time - key.time;
//			if (diffTime < 0.0) {
//				diffTime += duration;
//			}
//			if (diffTime > 0.0) {
//				float factor = (float)((time - key.time) / diffTime);
//				Position = key.vec + (nextKey.vec - key.vec) * factor;
//			}
//			else {
//				Position = key.vec;
//			}
//			std::get<0>(lastPositions[i]) = frame;
//
//		}
//		// interpolate rotation keyframes
//		SQuaternion rot;
//		if (channels[i].rotations.size() > 0) {
//			unsigned int frame = (time >= lastTime) ? std::get<1>(lastPositions[i]) : 0;
//			while (frame < channels[i].rotations.size() - 1) {
//				if (time < channels[i].rotations[frame + 1].time) {
//					break;
//				}
//				frame++;
//			}
//			if (frame >= channels[i].rotations.size()) {
//				frame = 0;
//			}
//			unsigned int nextFrame = (frame + 1) % channels[i].rotations.size();
//
//			auto key = channels[i].rotations[frame];
//			auto nextKey = channels[i].rotations[nextFrame];
//			key.quat.Normalize();
//			nextKey.quat.Normalize();
//			double diffTime = nextKey.time - key.time;
//			if (diffTime < 0.0) {
//				diffTime += duration;
//			}
//			if (diffTime > 0) {
//				float factor = (float)((time - key.time) / diffTime);
//				rot = SMath::Slerp(key.quat, nextKey.quat, factor);
//			}
//			else {
//				rot = key.quat;
//			}
//			// TODO
//			std::get<1>(lastPositions[i]) = frame;
//
//		}
//		// interpolate scale keyframes
//		SVector3 scale;
//		if (channels[i].scales.size() > 0) {
//			unsigned int frame = (time >= lastTime) ? std::get<2>(lastPositions[i]) : 0;
//			while (frame < channels[i].scales.size() - 1) {
//				if (time < channels[i].scales[frame + 1].time) {
//					break;
//				}
//				frame++;
//			}
//			if (frame >= channels[i].scales.size()) {
//				frame = 0;
//			}
//			std::get<2>(lastPositions[i]) = frame;
//		}
//
//		// create the combined transformation matrix
//		SMatrix mat(rot.RotationMatrix());
//		mat.m[0][0] *= scale.x; mat.m[1][0] *= scale.x; mat.m[2][0] *= scale.x;
//		mat.m[0][1] *= scale.y; mat.m[1][1] *= scale.y; mat.m[2][1] *= scale.y;
//		mat.m[0][2] *= scale.z; mat.m[1][2] *= scale.z; mat.m[2][2] *= scale.z;
//		mat.m[0][3] = Position.x; mat.m[1][3] = Position.y; mat.m[2][3] = Position.z;
//
//		bones[channels[i].name]->localTransform = mat;
//	}
//	lastTime = time;
//}
