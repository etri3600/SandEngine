#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <d3dcompiler.h>

#define RELEASE_DX(ptr) {if(ptr) ptr->Release(); ptr = nullptr;}

class SDirectX12Device
{
public:
	bool Initialize(unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync);
	void Finalize();

	ID3D12Device* GetDevice() const { return m_pDevice; }
	IDXGIFactory4* GetDXGIFactory() const { return m_pDXGIFactory; }

	unsigned int GetDeviceNumerator() const { return m_numerator; }
	unsigned int GetDeviceDenominator() const { return m_denominator; }

private:
	ID3D12Device* m_pDevice;
	IDXGIFactory4* m_pDXGIFactory;

	size_t m_videoCardMemory;
	unsigned int m_numerator, m_denominator;

#if defined(_DEBUG)
	ID3D12Debug* m_pDebugController;
#endif
};