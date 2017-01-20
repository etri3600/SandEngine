#include "SNode.h"
#include "STime.h"

void SNode::Tick()
{
	if (m_bDirty)
	{
		Draw();
		m_bDirty = false;
	}

	auto CurrentTime = STime::GetTime();
	auto milliseconds = m_Time > 0 ? CurrentTime - m_Time : 0LL;
	m_Time = CurrentTime;
	double delta = static_cast<double>(milliseconds) / 1000.0;
	UpdateObjects(delta);
	if (m_pGraphicsInterface)
	{
		m_pGraphicsInterface->UpdateBoneTransform(m_Models);
		m_pGraphicsInterface->Update(delta);
		m_pGraphicsInterface->Render();
		m_pGraphicsInterface->Present();
	}
}

void SNode::Queue(const SModel & model)
{
	m_Models.push_back(model);
}

void SNode::Draw()
{
	if (m_pGraphicsInterface)
	{
		m_pGraphicsInterface->Draw(m_Models);
	}
}

void SNode::Reset()
{
	m_Models.clear();
}

SNode* SNode::NextNode()
{
	SNode* pNode = nullptr;
	if (m_Children.size() > m_childIndex)
	{
		pNode = m_Children[m_childIndex];
		++m_childIndex;
	}
	else
		m_childIndex = 0;
	return pNode;
}

void SNode::UpdateObjects(double delta)
{
	for (auto it = m_Models.begin(); it != m_Models.end(); ++it)
	{
		it->Update(delta);
	}
}