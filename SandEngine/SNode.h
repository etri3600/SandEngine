#pragma once

#include <vector>

#include "SGraphicsInterface.h"

class SNode
{
public:
	void Tick();
	void Queue(const SModel& model);
	SNode* GetParent();
	void SetParent(SNode* pNode);
	SNode* CreateChild();
	void AddChild(SNode* pNode);
	void RemoveChild(SNode* pNode);
	std::vector<SNode*> GetChildren();
	void Draw();
	void Reset();

private:
	void UpdateObjects(double delta);

public:
	SNode* m_pParent;
	std::vector<SNode*> m_Children;

private:
	long long m_Time;
	std::vector<SModel> m_Models;
	bool m_bDirty = true;
};