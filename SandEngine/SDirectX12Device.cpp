#include "SDirectX12Device.h"
#include "SUtils.h"
#include <d3d12sdklayers.h>

SDirectX12Device::~SDirectX12Device()
{
	Finalize();
}

bool SDirectX12Device::Initialize(unsigned int screenWidth, unsigned int screenHeight, bool fullScreen, bool vSync)
{
	HRESULT hResult;
#if defined(_DEBUG)
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), reinterpret_cast<void**>(&m_pDebugController));
	if (SUCCEEDED(hResult))
	{
		m_pDebugController->EnableDebugLayer();
	}
#endif
	
	// Create DXGI
	IDXGIFactory4* factory = nullptr;
	IDXGIAdapter* adapter = nullptr;
	IDXGIOutput* adapterOutput = nullptr;
	hResult = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	factory->QueryInterface(IID_PPV_ARGS(&m_pDXGIFactory));

	// Create Device
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	hResult = D3D12CreateDevice(nullptr, featureLevel, __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_pDevice));
	if (FAILED(hResult))
	{
		m_pDXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		hResult = D3D12CreateDevice(adapter, featureLevel, __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_pDevice));
		if (FAILED(hResult))
		{
			return false;
		}
	}
	else
	{
		m_pDXGIFactory->EnumAdapters(0, &adapter);
		adapter->EnumOutputs(0, &adapterOutput);

	}

	unsigned int numerator = 0, denominator = 1;

	DXGI_ADAPTER_DESC adapterDesc;
	adapter->GetDesc(&adapterDesc);
	m_videoCardMemory = adapterDesc.DedicatedVideoMemory / 1024 / 1024;

	if (adapterOutput)
	{
		// Get DisplayInfo
		unsigned int numModes = 0;
		DXGI_MODE_DESC* displayModeList;
		adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		displayModeList = new DXGI_MODE_DESC[numModes];
		adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);

		for (unsigned int i = 0;i < numModes; ++i)
		{
			if (screenWidth == displayModeList[i].Width && screenHeight == displayModeList[i].Height)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
				break;
			}
		}

		if (numerator == 0 || denominator == 1)
		{
			for (unsigned int i = 0;i < numModes; ++i)
			{
				if (Sand::Equal(screenWidth / screenHeight, displayModeList[i].Width / displayModeList[i].Height))
				{
					numerator = displayModeList[i].RefreshRate.Numerator;
					denominator = displayModeList[i].RefreshRate.Denominator;
					break;
				}
			}
		}

		delete[] displayModeList;
		displayModeList = nullptr;

		adapterOutput->Release();
		adapterOutput = nullptr;
	}

	adapter->Release();
	adapter = nullptr;

	return true;
}

void SDirectX12Device::Finalize()
{
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
	
	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
		m_pDXGIFactory = nullptr;
	}
}
