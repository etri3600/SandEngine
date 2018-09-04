#pragma once

#include "Scene.h"
#include <vector>

class SSceneManager
{
public:
	SScene* CreateScene(wchar_t* name);

	void LoadScene(SScene* scene);
	void UnloadScene(SScene* scene);

	void Tick();

private:
	SScene* m_pCurScene;
	std::vector<SScene*> m_Scenes;
};