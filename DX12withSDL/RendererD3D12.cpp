#include "pch.h"
#include "RendererD3D12.h"

namespace GAL
{

    RendererD3D12::RendererD3D12()
    {
    }


    RendererD3D12::~RendererD3D12()
    {
    }

    bool RendererD3D12::Init(HWND hWnd)
    {
        //This example shows calling D3D12CreateDevice to create the device.
        //GAL: Minimum supported d3d version is 11. 
        HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
            __uuidof(ID3D12Device), (void**)&mDevice);

        D3D12_FEATURE_DATA_D3D12_OPTIONS options;
        if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options),
            sizeof(options))))
        {
            throw;
        }

        //Create the command allocator and queue objects.Then, obtain command lists from the command allocator and submit them to the command queue.
        //This example shows calling ID3D12Device::CreateCommandAllocator and ID3D12Device::GetDefaultCommandQueue. Latter doesn't exist tho??
        hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&mCommandListAllocator);
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
        commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = mDevice->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&mCommandQueue);

        //Create the swap chain similarly to how it was done in Direct3D 11.

        // Create the swap chain descriptor.
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = g_bbCount;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.Flags = 0; //DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        //swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;


        // Get the DXGI factory used to create the swap chain.
        IDXGIFactory2 *dxgiFactory = nullptr;
        hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&dxgiFactory);


        // Create the swap chain using the command queue, NOT using the device.  Thanks Killeak!
        hr = dxgiFactory->CreateSwapChain(mCommandQueue.Get(), &swapChainDesc, (IDXGISwapChain**)mSwapChain.GetAddressOf());

        //increase max frame latency when required
        if (swapChainDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            mSwapChain->SetMaximumFrameLatency(g_bbCount);
        }

        dxgiFactory->Release();

        //Create the RTV descriptor heap with g_bbCount entries (front and back buffer since swapchain resource rotation is no longer automatic).
        //Documentation recommends flip_sequential for D3D12, see - https://msdn.microsoft.com/en-us/library/windows/desktop/dn903945
        mRTVDescriptorHeap.Create(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, g_bbCount);

        //RTV descriptor and viewport/scissor initialization moved to seperate function
        hr = ResizeSwapChain();


        //create worldMatrix buffer resource on it's own heap and leave mapped, the end-of-frame fence prevents
        //it being written while the GPU is still reading it.
        UINT cbSize = sizeof(DirectX::XMMATRIX);
        hr = mWorldMatrix.Create(mDevice.Get(), cbSize, D3D12_HEAP_TYPE_UPLOAD);

        //create viewMatrix buffer resource on it's own heap and leave mapped
        hr = mViewMatrix.Create(mDevice.Get(), cbSize, D3D12_HEAP_TYPE_UPLOAD);

        //create projMatrix buffer resource on it's own heap and leave mapped
        hr = mProjMatrix.Create(mDevice.Get(), cbSize, D3D12_HEAP_TYPE_UPLOAD);

        //create the descriptor heap for the view and proj matrix CB views (and now a texture2d SRV view also)
        mCBDescriptorHeap.Create(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3, true);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc;
        //create the view matrix constant buffer view descriptor
        cbDesc.BufferLocation = mViewMatrix.pBuf->GetGPUVirtualAddress();
        cbDesc.SizeInBytes = 256;// max(cbSize, 256); //Size of 64 is invalid.  Device requires SizeInBytes be a multiple of 256. [ STATE_CREATION ERROR #645 ]
        mDevice->CreateConstantBufferView(&cbDesc, mCBDescriptorHeap.hCPU(0));

        //create the proj matrix constant buffer view descriptor
        cbDesc.BufferLocation = mProjMatrix.pBuf->GetGPUVirtualAddress();
        cbDesc.SizeInBytes = 256; //CB descriptor size must be multiple of 256
        mDevice->CreateConstantBufferView(&cbDesc, mCBDescriptorHeap.hCPU(1));
    }

    HRESULT RendererD3D12::ResizeSwapChain()
    {
        HRESULT hr = S_OK;
        g_requestResize = false;

        //get the client window area size.
        RECT clientSize;
        UINT width, height;
        GetClientRect(g_hWnd, &clientSize);
        width = clientSize.right;
        height = clientSize.bottom;

        //if the client size is valid (ignore minimize etc).
        if (width > 0)
        {

            //release existing backbuffer resources pointers (use reset for ComPtr rather than release, or just assign nullptr).
            for (UINT i = 0; i < g_bbCount; ++i)
            {
                mRenderTarget[i].Reset();
            }

            //resize the swapchain buffers
            mSwapChain->ResizeBuffers(g_bbCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0); //DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);

            //A buffer is required to render to.This example shows how to create that buffer by using the swap chain and device.
            //This example shows calling ID3D12Device::CreateRenderTargetView.
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
            ZeroMemory(&rtvDesc, sizeof(rtvDesc));
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            //loop for all backbuffer resources
            for (UINT i = 0; i < g_bbCount; ++i)
            {
                hr = mSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (LPVOID*)&mRenderTarget[i]);
                mRenderTarget[0]->SetName(L"mRenderTarget" + i);  //set debug name 
                mDevice->CreateRenderTargetView(mRenderTarget[i].Get(), &rtvDesc, mRTVDescriptorHeap.hCPU(i));
            }

            //fill out a viewport struct
            ZeroMemory(&mViewPort, sizeof(D3D12_VIEWPORT));
            mViewPort.TopLeftX = 0;
            mViewPort.TopLeftY = 0;
            mViewPort.Width = (float)width;
            mViewPort.Height = (float)height;
            mViewPort.MinDepth = 0.0f;
            mViewPort.MaxDepth = 1.0f;

            mRectScissor = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        }

        return hr;
    }

    void RendererD3D12::RenderFrame()
    {
        using namespace DirectX;

        HRESULT hr;

        if (g_requestResize) ResizeSwapChain();

        //rotation in radians of 0-90, 270-360 degrees (skip backfacing angles)
        static float angle = 0.0f;
        angle += XM_PI / 180.0f;
        if ((angle > XM_PI * 0.5f) && (angle < XM_PI * 1.5f)) angle = XM_PI * 1.5f;
        if (angle > XM_2PI) angle = 0.0f;

        //rotate worldmatrix around Y, transpose, and copy to the persistently mapped upload heap resource of the worldmatrix buffer
        XMMATRIX rotated = XMMatrixIdentity();
        rotated = XMMatrixRotationY(angle);
        rotated = XMMatrixTranspose(rotated);
        memcpy(mWorldMatrix.pDataBegin, &rotated, sizeof(rotated));

        //writing the view/proj buffers every frame, even tho they don't change in this example.
        //build and copy viewmatrix to the persistently mapped viewmatrix buffer
        XMVECTOR eye{ 0.0f, 0.0f, -2.0f, 0.0f };
        XMVECTOR eyedir{ 0.0f, 0.0f, 0.0f, 0.0f };
        XMVECTOR updir{ 0.0f, 1.0f, 0.0f, 0.0f };
        XMMATRIX view = XMMatrixLookAtLH(eye, eyedir, updir);
        view = XMMatrixTranspose(view);
        memcpy(mViewMatrix.pDataBegin, &view, sizeof(view));

        //build and copy projection matrix to the persistently mapped projmatrix buffer
        XMMATRIX proj = XMMatrixPerspectiveFovLH((XM_PI / 4.0f), (6.0f / 8.0f), 0.1f, 100.0f);
        proj = XMMatrixTranspose(proj);
        memcpy(mProjMatrix.pDataBegin, &proj, sizeof(proj));

        //Get the index of the active back buffer from the swapchain
        UINT backBufferIndex = 0;
        backBufferIndex = mSwapChain->GetCurrentBackBufferIndex();


        //get active RTV from descriptor heap wrapper class by index
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = mRTVDescriptorHeap.hCPU(backBufferIndex);


        //Now, reuse the command list for the current frame.
        //Reattach the viewport to the command list, indicate that the resource will be in use as a render target, record commands, and then 
        //indicate that the render target will be used to present when the command list is done executing.

        //This example shows calling ID3D12GraphicsCommandList::ResourceBarrier to indicate to the system that you are about to use a resource.
        //Resource barriers are used to handle multiple accesses to a resource(refer to the Remarks for ResourceBarrier).
        //You have to explicitly state that mRenderTarget is about to be changed from being "used to present" to being "used as a render target".
        mCommandList->RSSetViewports(1, &mViewPort);
        mCommandList->RSSetScissorRects(1, &mRectScissor);
        mCommandList->SetPipelineState(g_PSO.Get());
        mCommandList->SetGraphicsRootSignature(g_RootSig.Get());

        //set the root CBV of the worldmatrix and the root descriptor table containing the view and proj matrices' view descriptors
        mCommandList->SetGraphicsRootConstantBufferView(0, mWorldMatrix.pBuf->GetGPUVirtualAddress());
        ID3D12DescriptorHeap* pHeaps[2] = { mCBDescriptorHeap.pDH.Get(), mSamplerHeap.pDH.Get() };
        mCommandList->SetDescriptorHeaps(2, pHeaps); //this call IS necessary
        mCommandList->SetGraphicsRootDescriptorTable(1, mCBDescriptorHeap.hGPUHeapStart);
        //set the SRV and sampler tables
        mCommandList->SetGraphicsRootDescriptorTable(2, mCBDescriptorHeap.hGPU(2)); //the single SRV was put on the end of the CB heap
        mCommandList->SetGraphicsRootDescriptorTable(3, mSamplerHeap.hGPUHeapStart);

        // Indicate that this resource will be in use as a render target.
        setResourceBarrier(mCommandList.Get(), mRenderTarget[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        // Record commands.
        float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        mCommandList->ClearRenderTargetView(rtv, clearColor, NULL, 0);
        mCommandList->OMSetRenderTargets(1, &rtv, TRUE, nullptr);
        mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mCommandList->IASetVertexBuffers(0, 1, &g_VB.GetView());
        mCommandList->DrawInstanced(3, 1, 0, 0);

        // Indicate that the render target will now be used to present when the command list is done executing.
        setResourceBarrier(mCommandList.Get(), mRenderTarget[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        // Execute the command list.
        hr = mCommandList->Close();
        mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)mCommandList.GetAddressOf());


        // Swap the back and front buffers.
        DXGI_PRESENT_PARAMETERS params;
        ZeroMemory(&params, sizeof(params));
        //hr = mSwapChain->Present1(0, DXGI_PRESENT_DO_NOT_WAIT, &params);
        hr = mSwapChain->Present(0, 0);

        //wait for GPU to signal it has finished processing the queued command list(s).
        WaitForCommandQueueFence();


        // Command list allocators can be only be reset when the associated command lists have finished execution on the GPU; 
        // apps should use fences to determine GPU execution progress.
        hr = mCommandListAllocator->Reset();
        hr = mCommandList->Reset(mCommandListAllocator.Get(), NULL);
    }

}; //namespace GAL