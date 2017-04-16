#include <Windows.h>

#include "SScene.h"
#include "STime.h"

extern SIGraphicsInterface* gGraphics;

void SScene::Tick()
{
	SNode* pNode = m_pRootNode;
	pNode->Tick();
	if (m_Dirty)
	{
		gGraphics->Draw();
		m_Dirty = false;
	}
	gGraphics->Render();
	gGraphics->Present();
}
