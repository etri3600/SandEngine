#pragma once

#include <vector>

#include "SGraphicsInterface.h"

class SNode
{
public:
	void Tick();
	void Queue(const SModel& model);
	void Draw();
	void Reset();

	SNode* NextNode();

private:
	void UpdateObjects(double delta);

public:
	SNode* m_pParent;
	std::vector<SNode*> m_Children;

private:
	long long m_Time;
	SIGraphicsInterface* m_pGraphicsInterface = nullptr;
	std::vector<SModel> m_Models;
	unsigned int m_childIndex = 0;
	bool m_bDirty = true;
};