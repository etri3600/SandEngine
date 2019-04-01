#include "DX12DeferredRendering.h"
#include "DirectX12Device.h"
#include "DirectX12.h"
#include "DX12Pipeline.h"
#include "Platform/Windows.h"

#include <sstream>

SDX12DeferredRendering::SDX12DeferredRendering(class SDirectX12Device* Device)
	: m_pDevice(Device)
	, PostProcessRTV(Device)
	, PostProcessSRV(Device)
{
	// Create Shader buffer
	CreateGBuffers();
	CreatePostProcessResources();
	CreateRenderingPipeline();

	// Create Sampler
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
	descHeapSampler.NumDescriptors = 1;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	HRESULT hResult = m_pDevice->GetDevice()->CreateDescriptorHeap(&descHeapSampler, IID_PPV_ARGS(&m_pSamplerHeap));
	SWindows::OutputErrorMessage(hResult);

	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	m_pDevice->GetDevice()->CreateSampler(&samplerDesc, m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart());
}

SDX12DeferredRendering::~SDX12DeferredRendering()
{
	for (SDX12Pipeline*& pipeline : m_pipelines)
	{
		delete pipeline;
	}
	m_pipelines.clear();
}

void SDX12DeferredRendering::Render()
{
	HRESULT hResult;
	// Setting Default, Upload Heap
	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_HEAP_PROPERTIES uploadHeapProperties = defaultHeapProperties;
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	for (auto& pipeline : m_pipelines)
	{
		auto it = m_SceneProxy.BatchProxies.find(pipeline->GetMaterialType());
		if (it != m_SceneProxy.BatchProxies.end())
		{
			SBatchProxy* pBatchProxy = &it->second;

			std::vector<SSkinnedModelVertex> vertices;
			std::vector<unsigned int> indices;
			for (unsigned int i = 0; i < pBatchProxy->ObjectProxies.size(); ++i)
			{
				vertices.insert(std::end(vertices), std::begin(pBatchProxy->ObjectProxies[i].Vertices), std::end(pBatchProxy->ObjectProxies[i].Vertices));
				indices.insert(std::end(indices), std::begin(pBatchProxy->ObjectProxies[i].Indices), std::end(pBatchProxy->ObjectProxies[i].Indices));
			}

			// Vertex Buffer
			ID3D12Resource* vertexBufferUpload = nullptr;

			D3D12_RESOURCE_DESC vertexBufferDesc;
			vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			vertexBufferDesc.Alignment = 0;
			vertexBufferDesc.Width = pBatchProxy->VertexSize;
			vertexBufferDesc.Height = 1;
			vertexBufferDesc.DepthOrArraySize = 1;
			vertexBufferDesc.MipLevels = 1;
			vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			vertexBufferDesc.SampleDesc.Count = 1;
			vertexBufferDesc.SampleDesc.Quality = 0;
			vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pVertexBuffer));
			SWindows::OutputErrorMessage(hResult);
			m_pVertexBuffer->SetName(L"VertexBuffer");

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBufferUpload));
			SWindows::OutputErrorMessage(hResult);

			vertexBufferUpload->SetName(L"VertexBufferUpload");

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = vertices.data();
			vertexData.RowPitch = vertices.size() * sizeof(SSkinnedModelVertex);
			vertexData.SlicePitch = vertexData.RowPitch;

			UpdateSubresource(m_pCommandList, m_pVertexBuffer, vertexBufferUpload, 0, 0, 1, &vertexData);

			D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier;
			vertexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			vertexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			vertexBufferResourceBarrier.Transition.pResource = m_pVertexBuffer;
			vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			vertexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_pCommandList->ResourceBarrier(1, &vertexBufferResourceBarrier);


			// Index Buffer
			ID3D12Resource* indexbufferUpload = nullptr;
			D3D12_RESOURCE_DESC indexBufferDesc;
			indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			indexBufferDesc.Alignment = 0;
			indexBufferDesc.Width = pBatchProxy->IndexSize;
			indexBufferDesc.Height = 1;
			indexBufferDesc.DepthOrArraySize = 1;
			indexBufferDesc.MipLevels = 1;
			indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			indexBufferDesc.SampleDesc.Count = 1;
			indexBufferDesc.SampleDesc.Quality = 0;
			indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pIndexBuffer));
			SWindows::OutputErrorMessage(hResult);
			m_pIndexBuffer->SetName(L"IndexBuffer");

			hResult = m_pDevice->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexbufferUpload));
			SWindows::OutputErrorMessage(hResult);

			indexbufferUpload->SetName(L"IndexBufferUpload");

			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = indices.data();
			indexData.RowPitch = indices.size() * sizeof(unsigned int);
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresource(m_pCommandList, m_pIndexBuffer, indexbufferUpload, 0, 0, 1, &indexData);


			D3D12_RESOURCE_BARRIER indexBufferResourceBarrier;
			indexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			indexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			indexBufferResourceBarrier.Transition.pResource = m_pIndexBuffer;
			indexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			indexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
			indexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_pCommandList->ResourceBarrier(1, &indexBufferResourceBarrier);

			pipeline->SetVertexBufferView(m_pVertexBuffer->GetGPUVirtualAddress(), pBatchProxy->VertexSize, sizeof(SSkinnedModelVertex));
			pipeline->SetIndexBufferView(m_pIndexBuffer->GetGPUVirtualAddress(), pBatchProxy->IndexSize, DXGI_FORMAT_R32_UINT);
		}
	}

	auto srvHeap = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto srvSize = srvHeap->GetDescriptorSize();
	for (auto it = m_pipelines.begin(); it != m_pipelines.end(); ++it)
	{
		EMaterialType mat_type = (*it)->GetMaterialType();
		auto batchProxyIter = m_SceneProxy.BatchProxies.find(mat_type);
		// gbuffers
		if (batchProxyIter != m_SceneProxy.BatchProxies.end())
		{
			(*it)->CreateConstantBuffer(srvHeap, srvSize, &batchProxyIter->second);
			(*it)->CreateShaderResources(m_pCommandList, srvHeap, &batchProxyIter->second);
		}
		// others
		else
		{
			(*it)->CreateConstantBuffer(m_pGBufferCbvSrvHeap, srvSize, nullptr);
		}
	}

	// Execute Command
	m_pCommandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandList) / sizeof(ppCommandList[0]), ppCommandList);

	HRESULT hResult = GetCommandAllocator()->Reset();
	SWindows::OutputErrorMessage(hResult);

	hResult = m_pCommandList->Reset(GetCommandAllocator(), nullptr);
	SWindows::OutputErrorMessage(hResult);

	// GBuffer
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	D3D12_RECT ScissorRect = { 0L, 0L, static_cast<long>(m_Viewport.Width), static_cast<long>(m_Viewport.Height) };
	m_pCommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER gBufferResourceBarriers[static_cast<unsigned int>(EGBuffer::GB_NUM)] = {};
	gBufferResourceBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gBufferResourceBarriers[0].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_COLOR)];
	gBufferResourceBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	gBufferResourceBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gBufferResourceBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gBufferResourceBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gBufferResourceBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gBufferResourceBarriers[1].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_DEPTH)];
	gBufferResourceBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	gBufferResourceBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gBufferResourceBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gBufferResourceBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gBufferResourceBarriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gBufferResourceBarriers[2].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_NORMAL)];
	gBufferResourceBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	gBufferResourceBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gBufferResourceBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gBufferResourceBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(static_cast<UINT>(EGBuffer::GB_NUM), gBufferResourceBarriers);

	const float color[4] = { 0.1f, 0.5f, 0.7f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	renderTargetViewHandle.ptr = m_pGBufferRTVHeap->GetCPUDescriptorHandleForHeapStart().ptr;

	for (int i = 0; i < static_cast<int>(EGBuffer::GB_NUM); ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = renderTargetViewHandle.ptr + m_RenderTargetViewDescriptorSize * i;
		m_pCommandList->ClearRenderTargetView(handle, color, 0, nullptr);
	}
	m_pCommandList->OMSetRenderTargets(static_cast<UINT>(EGBuffer::GB_NUM), &renderTargetViewHandle, true, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto srvHeap = m_pDescriptorAllocator[int(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)]->GetOrAllocate()->GetHeapPointer();
	auto srvSize = m_pDescriptorAllocator[int(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)]->GetOrAllocate()->DescriptorSize();
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap };
	m_pCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ppHeaps[0]), ppHeaps);
	for (auto& pipeline : m_pipelines)
	{
		if (pipeline && pipeline->GetMaterialType() < EMaterialType::DEFERRED)
		{
			SBatchProxy* batchProxy = &m_SceneProxy.BatchProxies[pipeline->GetMaterialType()];
			auto signature = pipeline->GetRootSignature();

			m_pCommandList->IASetVertexBuffers(0, 1, &pipeline->GetVertexBufferView());
			m_pCommandList->IASetIndexBuffer(&pipeline->GetIndexBufferView());
			m_pCommandList->SetGraphicsRootSignature(pipeline->GetRootSignature()->Get());
			m_pCommandList->SetPipelineState(pipeline->GetPipelineState());
			{
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
				for (unsigned int i = 0; i < batchProxy->ObjectProxies.size(); ++i)
				{
					gpuHandle.ptr += pipeline->GetCBVDescriptorOffset();
					pipeline->Populate(m_pCommandList, i);
					pipeline->UpdateConstantBuffer(batchProxy, i, View, Projection);
					for (unsigned int j = 0; j < batchProxy->ObjectProxies[i].MeshProxy.size(); ++j)
					{
						m_pCommandList->SetGraphicsRootDescriptorTable(signature->GetTableLocation(), gpuHandle);
						m_pCommandList->DrawIndexedInstanced(batchProxy->ObjectProxies[i].MeshProxy[j].NumIndices, 1, batchProxy->ObjectProxies[i].StartIndexLocation + batchProxy->ObjectProxies[i].MeshProxy[j].BaseIndex, batchProxy->ObjectProxies[i].BaseVertexLocation + batchProxy->ObjectProxies[i].MeshProxy[j].BaseVertex, 0);
						gpuHandle.ptr += srvSize;
					}
				}
			}
		}
	}

	D3D12_RESOURCE_BARRIER gbuffersResourceBarriers[static_cast<unsigned int>(EGBuffer::GB_NUM)] = {};
	gbuffersResourceBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gbuffersResourceBarriers[0].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_COLOR)];
	gbuffersResourceBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gbuffersResourceBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	gbuffersResourceBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gbuffersResourceBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gbuffersResourceBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gbuffersResourceBarriers[1].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_DEPTH)];
	gbuffersResourceBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gbuffersResourceBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	gbuffersResourceBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gbuffersResourceBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	gbuffersResourceBarriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	gbuffersResourceBarriers[2].Transition.pResource = m_pGBuffers[static_cast<unsigned short>(EGBuffer::GB_NORMAL)];
	gbuffersResourceBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	gbuffersResourceBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	gbuffersResourceBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	gbuffersResourceBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(static_cast<UINT>(EGBuffer::GB_NUM), gbuffersResourceBarriers);

	// Light and Render
	D3D12_RESOURCE_BARRIER swapResourceBarrier;
	swapResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	swapResourceBarrier.Transition.pResource = GetRenderTarget();
	swapResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	swapResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	swapResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	swapResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	float backColor[4] = { 0, 1.0f, 1.0f, 1.0f };
	renderTargetViewHandle.ptr = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_BufferIndex * m_RenderTargetViewDescriptorSize;
	m_pCommandList->ClearRenderTargetView(renderTargetViewHandle, backColor, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &renderTargetViewHandle, false, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* ppDeferredHeaps[] = { m_pGBufferCbvSrvHeap };
	m_pCommandList->SetDescriptorHeaps(sizeof(ppDeferredHeaps) / sizeof(ppDeferredHeaps[0]), ppDeferredHeaps);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDeferredHandle = m_pGBufferCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();

	auto srvSize = m_pDescriptorAllocator[int(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)]->GetOrAllocate()->DescriptorSize();

	for (auto& pipeline : m_pipelines)
	{
		if (pipeline)
		{
			if (pipeline->GetMaterialType() == EMaterialType::DEFERRED)
			{
				auto signature = pipeline->GetRootSignature();
				m_pCommandList->IASetVertexBuffers(0, 1, &pipeline->GetVertexBufferView());
				m_pCommandList->IASetIndexBuffer(&pipeline->GetIndexBufferView());
				m_pCommandList->SetGraphicsRootSignature(pipeline->GetRootSignature()->Get());
				m_pCommandList->SetPipelineState(pipeline->GetPipelineState());

				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pGBufferCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
				gpuHandle.ptr += pipeline->GetCBVDescriptorOffset();

				m_pCommandList->SetGraphicsRootDescriptorTable(signature->GetTableLocation(), gpuHandle);
				m_pCommandList->DrawInstanced(6, 1, 0, 0);
				gpuHandle.ptr += srvSize;
			}
		}
	}

	swapResourceBarrier.Transition.pResource = GetRenderTarget();
	swapResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	swapResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	m_pCommandList->ResourceBarrier(1, &swapResourceBarrier);

	// PostProcess
	//m_pCommandList->RSSetViewports(1, &m_Viewport);
	//m_pCommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER postprocessResourceBarrier;
	postprocessResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	postprocessResourceBarrier.Transition.pResource = m_pPostProcessResource;
	postprocessResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	postprocessResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	postprocessResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	postprocessResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(1, &postprocessResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE postprocessRTVHandle;
	postprocessRTVHandle.ptr = m_pPostProcessRTVHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	m_pCommandList->ClearRenderTargetView(postprocessRTVHandle, color, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &postprocessRTVHandle, true, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* ppPostprocessHeaps[] = { m_pPostProcessCbvSrvHeap };
	m_pCommandList->SetDescriptorHeaps(sizeof(ppPostprocessHeaps) / sizeof(ppPostprocessHeaps[0]), ppPostprocessHeaps);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuPostprocessHandle = m_pPostProcessCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();

	m_pCommandList->IASetVertexBuffers(0, 0, nullptr);
	m_pCommandList->IASetIndexBuffer(nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto& pipeline : m_pipelines)
	{
		if (pipeline && pipeline->GetMaterialType() == EMaterialType::POSTPROCESS)
		{
			gpuPostprocessHandle.ptr += pipeline->GetCBVDescriptorOffset();
			m_pCommandList->SetGraphicsRootSignature(pipeline->GetRootSignature()->Get());
			m_pCommandList->SetPipelineState(pipeline->GetPipelineState());
			pipeline->Populate(m_pCommandList, 0);
			pipeline->UpdateConstantBuffer(nullptr, 0, View, Projection);
			m_pCommandList->SetGraphicsRootDescriptorTable(pipeline->GetRootSignature()->GetTableLocation(), gpuPostprocessHandle);
			m_pCommandList->DrawInstanced(6, 1, 0, 0);
		}
	}

	postprocessResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	postprocessResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_pCommandList->ResourceBarrier(1, &postprocessResourceBarrier);

	m_pCommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandLists) / sizeof(ppCommandLists[0]), ppCommandLists);
}

void SDX12DeferredRendering::CreateGBuffers()
{
	HRESULT hResult;

	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC gbufferDesc = {};
	gbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	gbufferDesc.Alignment = 0;
	gbufferDesc.Width = m_pDevice->m_ScreenWidth;
	gbufferDesc.Height = m_pDevice->m_ScreenHeight;
	gbufferDesc.DepthOrArraySize = 1;
	gbufferDesc.MipLevels = 1;
	gbufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	gbufferDesc.SampleDesc.Count = 1;
	gbufferDesc.SampleDesc.Quality = 0;
	gbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	gbufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Color[0] = 0.1f;
	clearValue.Color[1] = 0.5f;
	clearValue.Color[2] = 0.7f;
	clearValue.Color[3] = 1.0f;

	// todo format
	for (auto i = 0; i < static_cast<unsigned short>(EGBuffer::GB_NUM); ++i)
	{
		clearValue.Format = gbufferDesc.Format;
		hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &gbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &clearValue, IID_PPV_ARGS(&m_pGBuffers[i]));

		std::wostringstream name;
		name << L"GBuffer " << static_cast<EGBuffer>(i);
		m_pGBuffers[i]->SetName(name.str().c_str());
	}

	// RTV
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = static_cast<UINT>(EGBuffer::GB_NUM) + 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->Init(heapDesc);

	D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
	rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	rtDesc.Texture2D.PlaneSlice = 0;
	rtDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	for (int i = 0; i < static_cast<int>(EGBuffer::GB_NUM); ++i)
	{
		auto cpuHandle = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->GetHandleOffsetCPU(i);
		m_pDevice->GetDevice()->CreateRenderTargetView(m_pGBuffers[i], &rtDesc, cpuHandle);
	}

	// CBV_SRV_UAV
	heapDesc.NumDescriptors = DescriptorNum;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Init(heapDesc);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Texture2D.MipLevels = gbufferDesc.MipLevels;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	for (int i = 0; i < static_cast<int>(EGBuffer::GB_NUM); ++i) {
		auto cpuHandle = m_pDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHandleOffsetCPU(i);
		m_pDevice->GetDevice()->CreateShaderResourceView(m_pGBuffers[i], &srvDesc, cpuHandle);
	}
}

void SDX12DeferredRendering::CreatePostProcessResources()
{
	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 1;
	defaultHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC prostprocessDesc = {};
	prostprocessDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	prostprocessDesc.Alignment = 0;
	prostprocessDesc.Width = m_pDevice->m_ScreenWidth;
	prostprocessDesc.Height = m_pDevice->m_ScreenHeight;
	prostprocessDesc.DepthOrArraySize = 1;
	prostprocessDesc.MipLevels = 1;
	prostprocessDesc.Format = DXGI_FORMAT_R16_FLOAT;
	prostprocessDesc.SampleDesc.Count = 1;
	prostprocessDesc.SampleDesc.Quality = 0;
	prostprocessDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	prostprocessDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	HRESULT hResult = m_pDevice->GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &prostprocessDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pPostProcessResource));
	m_pPostProcessResource->SetName(L"PostProcess");

	// RTV
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	PostProcessRTV.Init(heapDesc);

	D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
	rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	rtDesc.Texture2D.PlaneSlice = 0;
	rtDesc.Format = DXGI_FORMAT_R16_FLOAT;

	m_pDevice->GetDevice()->CreateRenderTargetView(m_pPostProcessResource, &rtDesc, PostProcessRTV.GetHandleOffsetCPU(0));

	// CBV_SRV_UAV
	heapDesc.NumDescriptors = DescriptorNum;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	PostProcessSRV.Init(heapDesc);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Texture2D.MipLevels = prostprocessDesc.MipLevels;
	srvDesc.Format = DXGI_FORMAT_R16_FLOAT;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_pDevice->GetDevice()->CreateShaderResourceView(m_pPostProcessResource, &srvDesc, PostProcessSRV.GetHandleOffsetCPU(0));
}

void SDX12DeferredRendering::CreateRenderingPipeline()
{
	// Create PSO for GBuffers
	DXGI_FORMAT rtvFormats[static_cast<unsigned int>(EGBuffer::GB_NUM)] = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };

	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12BoneRootParameter()));
	m_pipelines.push_back(new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::SKINNING));
	m_pipelines.back()->Init(L"SkinnedShader", L"SkinnedShader", "vert", "frag", new SDX12SkinnedResources(), 3, rtvFormats);

	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12TextureRootParameter()));
	m_pipelines.push_back(new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::TEXTURE));
	m_pipelines.back()->Init(L"TextureVertexShader", L"TexturePixelShader", "main", "main", new SDX12TextureResources(), 3, rtvFormats);

	// Create PSO for Light Pass


	// Create PSO for Deferred Pass
	DXGI_FORMAT deferredFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12SsAoRootParameter()));

	SDX12Pipeline* deferredPipeline = new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::DEFERRED);
	deferredPipeline->Init(L"DeferredLighting", L"DeferredLighting", "vert", "frag", new SDX12DeferredResources(), 1, deferredFormats);
	m_pipelines.push_back(deferredPipeline);

	// Create PSO for Postprocess
	DXGI_FORMAT postprocessFormats[1] = { DXGI_FORMAT_R16_FLOAT };
	m_RootSignatures.push_back(new SDX12RootSignature(m_pDevice, new SDX12SsAoRootParameter()));

	SDX12Pipeline* postprocessPipeline = new SDX12Pipeline(m_pDevice, m_RootSignatures.back(), EMaterialType::POSTPROCESS);
	postprocessPipeline->Init(L"SsAo", L"SsAo", "vert", "frag", new SDX12SsAoResources(), 1, postprocessFormats);
	m_pipelines.push_back(postprocessPipeline);
}