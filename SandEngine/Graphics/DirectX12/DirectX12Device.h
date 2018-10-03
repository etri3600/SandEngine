#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include "Platform/PlatformSystem.h"

#define RELEASE_DX(ptr) {if(ptr) ptr->Release(); ptr = nullptr;}

class SDirectX12Device
{
public:
	~SDirectX12Device();

	bool Initialize(unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync);
	void Finalize();

	ID3D12Device* GetDevice() const { return m_pDevice; }
	IDXGIFactory5* GetDXGIFactory() const { return m_pDXGIFactory; }

	unsigned int GetDeviceNumerator() const { return m_numerator; }
	unsigned int GetDeviceDenominator() const { return m_denominator; }

public:
	unsigned int m_ScreenWidth, m_ScreenHeight;

private:
	ID3D12Device* m_pDevice;
	IDXGIFactory5* m_pDXGIFactory;

	size_t m_videoCardMemory;
	unsigned int m_numerator, m_denominator;

#if defined(_DEBUG)
	ID3D12Debug* m_pDebugController;
#endif
};
