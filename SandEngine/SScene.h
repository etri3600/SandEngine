#pragma once

#include "SGraphicsInterface.h"
#include "SNode.h"

class SScene
{
public:
	SNode* GetRootNode() { if (m_pRootNode == nullptr) m_pRootNode = new SNode(); return m_pRootNode; }

	void Init(wchar_t* name) { m_sceneName = name; }
	void Tick();

private:
	long long m_Time;
	SNode* m_pRootNode;
	wchar_t* m_sceneName;
};