#pragma once

#include "SGraphicsInterface.h"
#include "STime.h"

class SSceneManager
{
public:
	void Init(SIGraphicsInterface* pGraphicsInterface);
	void Tick();
	void Queue(const SModel& model);
	void Draw();
	void Reset();

private:
	void UpdateObjects(double delta);

	SIGraphicsInterface* m_pGraphicsInterface = nullptr;

	std::vector<SModel> m_Models;

	long long m_Time = 0LL;
};