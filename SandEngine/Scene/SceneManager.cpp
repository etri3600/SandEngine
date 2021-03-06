#include "SceneManager.h"
#include "STime.h"

extern IGraphicsInterface* gGraphics;

SScene* SSceneManager::CreateScene(wchar_t* name)
{
	auto scene = new SScene();
	scene->Init(name);
	m_Scenes.push_back(scene);

	return scene;
}

void SSceneManager::LoadScene(SScene* scene)
{
	UnloadScene(m_pCurScene);
	m_pCurScene = scene;
	gGraphics->CreateViewProjection();
}

void SSceneManager::UnloadScene(SScene* scene)
{
	if (scene != nullptr)
	{

	}
}

void SSceneManager::Tick()
{
	if (m_pCurScene)
	{
		m_pCurScene->Tick();
	}
}
