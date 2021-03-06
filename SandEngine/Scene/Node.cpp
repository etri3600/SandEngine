#include "Node.h"
#include "STime.h"

extern IGraphicsInterface* gGraphics;

void SNode::Tick()
{
	auto CurrentTime = STime::GetTime();
	auto milliseconds = m_Time > 0 ? CurrentTime - m_Time : 0LL;
	m_Time = CurrentTime;
	double delta = static_cast<double>(milliseconds) / 1000.0;

	if (m_bDirty)
	{
		Draw(delta);
		m_bDirty = false;
	}

	UpdateObjects(delta);
	gGraphics->UpdateBoneTransform(m_MaterialObjs);

	for (auto it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		(*it)->Tick();
	}
}

void SNode::Queue(SSceneObj* obj)
{
	if (obj)
	{
		m_MaterialObjs[obj->Material.Type].push_back(obj);
	}
}

SNode* SNode::GetParent()
{
	return m_pParent;
}

void SNode::SetParent(SNode * pNode)
{
	if (m_pParent)
	{
		m_pParent->RemoveChild(this);
	}

	m_pParent = pNode;
}

SNode* SNode::CreateChild()
{
	auto child = new SNode();
	child->SetParent(this);
	m_Children.push_back(child);
	return child;
}

void SNode::AddChild(SNode* pNode)
{
	m_Children.push_back(pNode);
}

void SNode::RemoveChild(SNode* pNode)
{
	auto cit = std::find(m_Children.cbegin(), m_Children.cend(), pNode);
	if(cit != m_Children.cend())
		m_Children.erase(cit);
}

std::vector<SNode*> SNode::GetChildren()
{
	return m_Children;
}

void SNode::Draw(double delta)
{
	gGraphics->Update(delta, m_MaterialObjs);
}

void SNode::Reset()
{
	m_MaterialObjs.clear();
}

void SNode::UpdateObjects(double delta)
{
	for (auto it = m_MaterialObjs.begin(); it != m_MaterialObjs.end(); ++it)
	{
		for (auto model_it = it->second.begin(); model_it != it->second.end(); ++model_it)
		{
			(*model_it)->Update(delta);
		}
	}
}