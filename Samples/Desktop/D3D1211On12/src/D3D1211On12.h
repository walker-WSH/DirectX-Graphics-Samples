//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSample.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

#include <windows.h>
#include <d2d1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwrite.h>
#include <wrl/client.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")

class D3D1211on12 : public DXSample
{
public:
    D3D1211on12(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 3;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
    ComPtr<ID3D11On12Device> m_d3d11On12Device;
    ComPtr<ID3D12Device> m_d3d12Device;
    ComPtr<IDWriteFactory> m_dWriteFactory;
    ComPtr<ID2D1Factory3> m_d2dFactory;
    ComPtr<ID2D1Device2> m_d2dDevice;
    ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D11Resource> m_wrappedBackBuffers[FrameCount];
    ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // App resources.
    UINT m_rtvDescriptorSize;
    ComPtr<ID2D1SolidColorBrush> m_textBrush;
    ComPtr<IDWriteTextFormat> m_textFormat;
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForGpu();
    void MoveToNextFrame();
    void RenderUI();

    ComPtr<ID2D1Bitmap1> sharedBitmap = nullptr;
    bool OpenSharedTexture(HANDLE sharedHandle)
    {
        /*
        ComPtr<ID3D11On12Device> m_d3d11On12Device;
        ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
        ComPtr<ID2D1Bitmap1> sharedBitmap;
        */
        if (!m_d3d11On12Device || !m_d2dDeviceContext)
            return false;

        sharedHandle = (HANDLE)0X40001E82LL;

        // 1. 获取 ID3D11Device*
        ComPtr<ID3D11Device> d3d11Device;
        auto hr = m_d3d11On12Device.As(&d3d11Device);
        assert(SUCCEEDED(hr));

        // 2. 打开共享句柄
        ComPtr<ID3D11Texture2D> sharedTex;
        hr = d3d11Device->OpenSharedResource(sharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sharedTex.GetAddressOf()));
        assert(SUCCEEDED(hr));

        D3D11_TEXTURE2D_DESC desc;
        sharedTex->GetDesc(&desc);
        assert(desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM);
        assert((desc.BindFlags & (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE)) != 0);
        assert(desc.SampleDesc.Count == 1);
        assert(desc.ArraySize == 1);

        Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
        hr = sharedTex.As(&dxgiSurface);
        assert(SUCCEEDED(hr));

        D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(desc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED);
        D2D1_BITMAP_PROPERTIES1 prt = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, pixelFormat);
        hr = m_d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), prt, &sharedBitmap);
        assert(SUCCEEDED(hr));

        return SUCCEEDED(hr);
    }

    /* 渲染d2d image
        D2D1_SIZE_F bitmapSize = sharedBitmap->GetSize();
        m_d2dDeviceContext->DrawBitmap(sharedBitmap.Get(), D2D1::RectF(0, 0, bitmapSize.width / 2, bitmapSize.height/ 2));
    */
};
