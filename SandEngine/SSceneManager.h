#pragma once

#include "SGraphicsInterface.h"

class SSceneManager
{
public:
	void Init(SIGraphicsInterface* pGraphicsInterface);
	void Tick(const double delta);
	void Queue(const SModel& model);
	void Draw();
	void Reset();

	SIGraphicsInterface* m_pGraphicsInterface;

	std::vector<SModel> m_Models;
};