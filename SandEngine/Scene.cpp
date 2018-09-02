#include <Windows.h>

#include "Scene.h"
#include "Time.h"

extern IGraphicsInterface* gGraphics;

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
