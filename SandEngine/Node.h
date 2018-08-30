#pragma once

#include <vector>
#include <map>

#include "GraphicsInterface.h"

class SNode
{
public:
	void Tick();
	void Queue(SSceneObj* obj);
	SNode* GetParent();
	void SetParent(SNode* pNode);
	SNode* CreateChild();
	void AddChild(SNode* pNode);
	void RemoveChild(SNode* pNode);
	std::vector<SNode*> GetChildren();
	void Draw(double delta);
	void Reset();

private:
	void UpdateObjects(double delta);

public:
	SNode* m_pParent;
	std::vector<SNode*> m_Children;

private:
	long long m_Time;
	std::map<EMaterialType, std::vector<SSceneObj*>> m_MaterialObjs;
	bool m_bDirty = true;
};