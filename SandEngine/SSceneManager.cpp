#include "SSceneManager.h"
#include <Windows.h>

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
	m_Models.clear();
}

void SSceneManager::UpdateObjects(double delta)
{
	for (auto it = m_Models.begin(); it != m_Models.end(); ++it)
	{
		it->Update(delta);
	}
}
