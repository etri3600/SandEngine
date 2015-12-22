#include "SSceneManager.h"

void SSceneManager::Init(SIGraphicsInterface * pGraphicsInterface)
{
	m_pGraphicsInterface = pGraphicsInterface;
	if (m_pGraphicsInterface)
	{
		m_pGraphicsInterface->CreateViewProjection();
	}
}

void SSceneManager::Tick()
{
	auto delta = m_Time > 0 ? STime::GetTime() - m_Time : 0LL;
	if (m_pGraphicsInterface)
	{
		m_pGraphicsInterface->Update(delta);
	}
	m_Time = STime::GetTime();
}

void SSceneManager::Queue(const SModel & model)
{
	m_Models.push_back(model);
}

void SSceneManager::Draw()
{
	if (m_pGraphicsInterface)
	{
		m_pGraphicsInterface->Draw(m_Models);
	}
}

void SSceneManager::Reset()
{
	m_Models.empty();
}
