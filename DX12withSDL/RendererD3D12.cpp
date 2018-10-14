#include "pch.h"
#include "RendererD3D12.h"

namespace GAL
{
#define MY_RENDER_TARGET_FORMAT (DXGI_FORMAT_R8G8B8A8_UNORM)
#define MY_SAMPLE_DESC_COUNT (1)
#define MY_SAMPLE_DESC_QUALITY (0)

    RendererD3D12::RendererD3D12()
    {
    }


    RendererD3D12::~RendererD3D12()
    {
    }

    bool RendererD3D12::Init(HWND hWnd)
    {
        bool retVal = false;

        //get the client window area size.
        RECT clientSize;;
        GetClientRect(hWnd, &clientSize);
        m_windowWidth = clientSize.right;
        m_windowHeight = clientSize.bottom;

        do {
            if (!CreateDeviceAndSwapChain())
            {
                break;
            }
            if (!CreateAllocatorsAndCommandLists())
            {
                break;
            }
            DefineViewportScissor();
            if (!CreateRootSignature())
            {
                break;
            }
            if (!CreatePipelineStateObject())
            {
                break;
            }
            retVal = true;
        } while (0);

        return retVal;
    }

    bool RendererD3D12::SetupRenderTargets()
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = kBackBufferCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_renderTargetDescriptorHeap));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{
            m_renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

        D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
        ZeroMemory(&viewDesc, sizeof(viewDesc));
        viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        for (int i = 0; i < kBackBufferCount; ++i) {

            m_device->CreateRenderTargetView(m_renderTargets[i].Get(), &viewDesc,
                rtvHandle);
            rtvHandle.Offset(m_renderTargetViewDescriptorSize);
        }

        return true;
    }


    bool RendererD3D12::SetupSwapChain()
    {
        // This is the first fence value we'll set, has to be != our initial value
        // below so we can wait on the first fence correctly
        m_currentFenceValue = 1;

        // Create fences for each frame so we can protect resources and wait for
        // any given frame
        for (int i = 0; i < kBackBufferCount; ++i) {
            m_frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            m_fenceValues[i] = 0;
            m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                IID_PPV_ARGS(&m_frameFences[i]));
        }

        for (int i = 0; i < kBackBufferCount; ++i) {
            m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
            m_renderTargets[i]->SetName(L"mRenderTarget" + i);  //set debug name 
        }

        return SetupRenderTargets();
    }

    bool RendererD3D12::CreateDeviceAndSwapChain()
    {
        // Enable the debug layers when in debug mode
    // If this fails, install the Graphics Tools for Windows. On Windows 10,
    // open settings, Apps, Apps & Features, Optional features, Add Feature,
    // and add the graphics tools
#ifdef _DEBUG
        ComPtr<ID3D12Debug> debugController;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

        if (debugController) {
            debugController->EnableDebugLayer();
        }
#endif

        //This example shows calling D3D12CreateDevice to create the device.
        //GAL: Minimum supported d3d version is 11. 
        HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
            __uuidof(ID3D12Device), (void**)&mDevice.Get());

        D3D12_FEATURE_DATA_D3D12_OPTIONS options;
        if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options),
            sizeof(options))))
        {
            ODERROR("The device doesn't support D3d12 options");
            return false;
        }

        //Create the Command Queue used for Rendering. There can be Command Queue for Computing and for Copying.
        //This one is for rendering.
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
        commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&&mCommandQueue));
        if (FAILED(hr)) {
            ODERROR("Command queue creation failed.");
            return false;
        }

        //Create the swap chain similarly to how it was done in Direct3D 11.

        // Create the swap chain descriptor.
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = RendererD3D12::kBackBufferCount;
        swapChainDesc.BufferDesc.Format = MY_RENDER_TARGET_FORMAT;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.SampleDesc.Count = MY_SAMPLE_DESC_COUNT;
        swapChainDesc.SampleDesc.Quality = MY_SAMPLE_DESC_QUALITY;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.Flags = 0; //DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        //Documentation recommends flip_sequential for D3D12, see - https://msdn.microsoft.com/en-us/library/windows/desktop/dn903945
        //swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;


        // Get the DXGI factory used to create the swap chain.
        ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
        if (FAILED(hr))
        {
            ODERROR("DXGI factory creation failed.");
            return false;
        }


        // Create the swap chain using the command queue, NOT using the device.  Thanks Killeak!
        hr = dxgiFactory->CreateSwapChain(mCommandQueue.Get(), &swapChainDesc, (IDXGISwapChain**)mSwapChain.GetAddressOf());

        //increase max frame latency when required
        if (swapChainDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            mSwapChain->SetMaximumFrameLatency(RendererD3D12::kBackBufferCount);
        }

        m_renderTargetViewDescriptorSize =
            device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        return SetupSwapChain();
    }

    bool RendererD3D12::CreateAllocatorsAndCommandLists()
    {
        for (int i = 0; i < kBackBufferCount; ++i) {
            m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_commandAllocators[i]));
            device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_commandAllocators[i].Get(), nullptr,
                IID_PPV_ARGS(&m_commandLists[i]));
            m_commandLists[i]->Close();
        }
        return true;
    }

   void RendererD3D12::DefineViewportScissor()
    {
        m_rectScissor = { 0, 0, m_windowWidth, m_windowHeight };

        m_viewport = { 0.0f, 0.0f,
            static_cast<float>(m_windowWidth),
            static_cast<float>(m_windowHeight),
            0.0f, 1.0f
        };
    }

   bool RendererD3D12::CreateRootSignature()
   {
       //This is the most simple root signature, because there are no parameters, no matrices,
       //no textures, etc. Only the vertices data are relevant.
       ComPtr<ID3DBlob> rootBlob;
       ComPtr<ID3DBlob> errorBlob;

       // We don't use another descriptor heap for the sampler, instead we use a
       // static sampler
       CD3DX12_STATIC_SAMPLER_DESC samplers[1];
       samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

       CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;

       // Create the root signature
       descRootSignature.Init(0, nullptr,
           1, samplers,
           D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
           D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
           D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
           D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

       D3D12SerializeRootSignature(&descRootSignature,
           D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

       m_device->CreateRootSignature(0,
           rootBlob->GetBufferPointer(),
           rootBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

       return true;
   }

   bool RendererD3D12::CreatePipelineStateObject()
   {
       //Create vertex and pixel shader
       m_vertexShader = std::make_unique<Shader>("P3F_C4F_Shaders.hlsl", "VSMain", Shader::TargetType::VertexShader);
       if (m_vertexShader->GetCompileError()) {
           ODERROR("Failed to compile vertex shader: %s", m_vertexShader->GetCompileError());
           return false;
       }

       m_pixelShader = std::make_unique<Shader>("P3F_C4F_Shaders.hlsl", "PSMain", Shader::TargetType::PixelShader);
       if (m_pixelShader->GetCompileError()) {
           ODERROR("Failed to compile pixel shader: %s", m_pixelShader->GetCompileError());
           return false;
       }


       //FIXME: PSO should not be created/owned by render nodes.
       // create a pipeline state object (PSO)

       // In a real application, you will have many pso's. for each different shader
       // or different combinations of shaders, different blend states or different rasterizer states,
       // different topology types (point, line, triangle, patch), or a different number
       // of render targets you will need a pso

       // VS is the only required shader for a pso. You might be wondering when a case would be where
       // you only set the VS. It's possible that you have a pso that only outputs data with the stream
       // output, and not on a render target, which means you would not need anything after the stream
       // output.
       D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
       psoDesc.InputLayout = P3F_C4F::GetInputLayoutDesc(); // the structure describing our input layout
       psoDesc.pRootSignature = m_rootSignature.Get(); // the root signature that describes the input data this pso needs
       psoDesc.VS = m_vertexShader->GetCompiledByteCode(); // structure describing where to find the vertex shader bytecode and how large it is
       psoDesc.PS = m_pixelShader->GetCompiledByteCode(); // same as VS but for pixel shader
       psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
       psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
       psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blend state.
       psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // format of the render target
       psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
       psoDesc.NumRenderTargets = 1; // we only have one format in RTVFormats[8]
       psoDesc.SampleDesc.Count = MY_SAMPLE_DESC_COUNT; // must be the same sample description as the swapchain and depth/stencil buffer
       psoDesc.SampleDesc.Quality = MY_SAMPLE_DESC_QUALITY;
       psoDesc.DepthStencilState.DepthEnable = false;
       psoDesc.DepthStencilState.StencilEnable = false;
       psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done

       // create the pso
       hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject.Get()));
       if (FAILED(hr))
       {
           ODERROR("Failed to create PSO: result=0x%08X", hr);
           return false;
       }

       return true;
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