#pragma once

#include <map>
#include <queue>

#include "SModelStruct.h"

#define TICKS_PER_SECOND 30.0

struct SAnimTimelineVector
{
	double time;
	SVector3 vec;
};

struct SAnimTimelineQuat
{
	double time;
	SQuaternion quat;
};

struct SAnimChannel
{
	std::string boneName = "";
	std::vector<SAnimTimelineVector> positions;
	std::vector<SAnimTimelineQuat> rotations;
	std::vector<SAnimTimelineVector> scales;
};

class SAnimInfo
{
public:
	std::string GetName() const { return name; }

protected:
	std::string name;
	std::vector<SAnimChannel> channels;
	std::vector<std::tuple<int, int, int>> lastPositions;
	double tickPerSeconds;
	double duration;

	double lastTime;
	std::vector<std::vector<SMatrix>> transforms;

	friend class SAnimation;
	friend class SModelLoader;
};

class SBoneNode;
class SBone;
class SAnimation
{
public:
	bool HasSkeleton() const { return m_Bones.size() > 0; }
	bool HasAnimation() const { return m_Animations.size() > 0; }
	std::string AnimName() const { return m_Animations[m_CurrentAnimIndex].name; }
	double AnimSpeed() const { return m_Animations[m_CurrentAnimIndex].tickPerSeconds; }
	double AnimDuration() const { return m_Animations[m_CurrentAnimIndex].duration / AnimSpeed(); }
	std::vector<SMatrix> GetTransform() const { return Transforms; }
	bool SetAnimation(const std::string clipName);
	void SetAnimationIndex(const unsigned int index) { if(m_Animations.size() > index) m_CurrentAnimIndex = index; }
	SAnimInfo& GetCurrentAnimInfo() { return m_Animations[m_CurrentAnimIndex]; }

	void Update(double delta);
	void CalculateBoneNodeToWorldTransform(SBoneNode* child);

private:
	void ReadNodeHeirarchy(double AnimationTime, const SBoneNode* pNode, const SMatrix& ParentTransform);
	bool FindNodeChannel(const SAnimInfo* pAnimInfo, const std::string& boneName, SAnimChannel& channel);

	void CalcInterpolatedScaling(SVector3& Out, double AnimationTime, const SAnimChannel& channel);
	void CalcInterpolatedRotation(SQuaternion& Out, double AnimationTime, const SAnimChannel& channel);
	void CalcInterpolatedPosition(SVector3& Out, double AnimationTime, const SAnimChannel& channel);

protected:
	double m_ElapsedTime = 0.0;

	SBoneNode* m_Skeleton = nullptr;
	std::map<std::string, SBoneNode*> m_BoneNodeNameMap;

	std::vector<SBone*> m_Bones;
	std::map<std::string, unsigned int> m_BoneNameMap;
	unsigned int m_NumBones = 0;

	std::map<std::string, unsigned int> m_AnimNameMap;
	std::vector<SAnimInfo> m_Animations;
	unsigned int m_CurrentAnimIndex = 0;

	std::vector<SMatrix> Transforms;
	SMatrix GlobalInverseTransformation;

	friend class SModelLoader;
	friend class SModel;
};