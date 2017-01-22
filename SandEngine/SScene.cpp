#include <Windows.h>

#include "SScene.h"
#include "STime.h"

void SScene::Tick()
{
	SNode* pNode = m_pRootNode;
	pNode->Tick();
}
