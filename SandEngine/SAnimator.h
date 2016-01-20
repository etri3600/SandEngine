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
	std::string name;
	std::vector<SAnimTimelineVector> positions;
	std::vector<SAnimTimelineQuat> rotations;
	std::vector<SAnimTimelineVector> scales;
};

class SAnimInfo
{
public:
	std::vector<SMatrix> GetTransform(double delta) { return transforms[GetFrameIndexAt(delta)]; }
	unsigned int GetFrameIndexAt(double delta)	{
		delta *= tickPerSeconds;
		double time = 0.0f;
		if (duration > 0.0f)
		{
			time = std::fmod(delta, duration);
		}
		auto percent = time / duration;
		return static_cast<unsigned int>(transforms.size() * percent);
	}
	std::string GetName() const { return name; }
	void Evaluate(double delta, std::map<std::string, SBone*> bones);

protected:
	std::string name;
	std::vector<SAnimChannel> channels;
	double lastTime;
	double tickPerSeconds;
	double duration;
	std::vector<std::tuple<int, int, int>> lastPositions;
	std::vector<std::vector<SMatrix>> transforms;

	friend class SAnimation;
	friend class SModelLoader;
};

class SAnimation
{
public:
	bool HasSkeleton() const { return m_Bones.size() > 0; }
	bool HasAnimation() const { return m_Animations.size() > 0; }
	std::string AnimName() const { return m_Animations[m_CurrentAnimIndex].name; }
	double AnimSpeed() const { return m_Animations[m_CurrentAnimIndex].tickPerSeconds; }
	double AnimDuration() const { return m_Animations[m_CurrentAnimIndex].duration / AnimSpeed(); }
	std::vector<SMatrix> GetTransform(double delta) { return m_Animations[m_CurrentAnimIndex].GetTransform(delta); }
	bool SetAnimation(const std::string clipName);
	void SetAnimationIndex(const unsigned int index) { if(m_Animations.size() > index) m_CurrentAnimIndex = index; }

	void ExtractAnimation();
	void CalculateAnimTime(double delta);
	void UpdateBoneTransform(SBone* bone);
	void CalculateBoneToWorldTransform(SBone* child);

protected:
	SBone* m_Skeleton = nullptr;
	std::map<std::string, SBone*> m_BoneNameMap;
	std::map<std::string, unsigned int> m_BoneIndex;
	std::map<std::string, unsigned int> m_AnimNameIdMap;
	std::vector<SBone*> m_Bones;
	std::vector<SAnimInfo> m_Animations;

	unsigned int m_CurrentAnimIndex = 0;

	friend class SModelLoader;
	friend class SAnimator;
};

class SAnimator
{
public:
	SAnimator(std::string clipName, SMatrix world, SModel model)
		:m_clipName(clipName), m_world(world), m_model(model)
	{}

	void Update(double delta);

	std::string GetClipName() const { return m_clipName; }
	void SetClipName(const std::string clipName);
	std::vector<std::string> GetClips() const;
	void AddClip(const std::string& clip) { m_clipQueue.push(clip); }
	void ClearClip() { std::queue<std::string> empty; std::swap(m_clipQueue, empty); }

	bool HasAnimation() const { return m_model.Animation->HasAnimation(); }
	std::vector<SMatrix> GetFinalTransform() { return m_model.Animation->GetTransform(m_timePoint); }

	SModel m_model;
	double m_timePoint;
	SMatrix m_world;
	std::string m_clipName;
	std::queue<std::string> m_clipQueue;
	bool m_bLoopClips;

};