#pragma once

#include <map>

#include "SModelStruct.h"

struct SAnimTimelineVector
{
	float time;
	SVector3 vec;
};

struct SAnimTimelineQuat
{
	float time;
	SQuaternion quat;
};

struct SAnimChannel
{
	std::string name;
	std::vector<SAnimTimelineVector> positions;
	std::vector<SAnimTimelineQuat> rotations;
	std::vector<SAnimTimelineVector> scales;
};

struct SAnimInfo
{
	std::string name;
	std::vector<SAnimChannel> channels;
	float lastTime;
	float tickPerSeconds;
	float duration;
	std::vector<std::tuple<int, int, int>> lastPositions;
	std::vector<std::vector<SMatrix>> transforms;
};

class SAnimator
{
public:
	bool HasSkeleton() const { return m_Bones.size() > 0; }
	std::string AnimName() const { return m_Animations[m_CurrentAnimIndex].name; }
	float AnimSpeed() const { return m_Animations[m_CurrentAnimIndex].tickPerSeconds; }
	float AnimDuration() const { return m_Animations[m_CurrentAnimIndex].duration / AnimSpeed(); }

protected:
	SBone m_Skeleton;
	std::map<std::string, SBone> m_BoneNameMap;
	std::map<std::string, unsigned int> m_BoneIndex;
	std::map<std::string, unsigned int> m_AnimNameIdMap;
	std::vector<SBone> m_Bones;
	std::vector<SAnimInfo> m_Animations;

	unsigned int m_CurrentAnimIndex = 0;

	friend class SModelLoader;
};