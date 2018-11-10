#include "pch.h"

#include "LogUtils.h"

#include "d3d12renderer/GpuResourceTypes/GpuMesh.h"
#include "d3d12renderer/D3D12Renderer.h"


#include "RenderNode.h"
#include "Shader.h"
#include "VertexTypes.h"
#include "Camera.h"

namespace GAL
{
#define MY_RENDER_TARGET_FORMAT (DXGI_FORMAT_R8G8B8A8_UNORM)
#define MY_SAMPLE_DESC_COUNT (1)
#define MY_SAMPLE_DESC_QUALITY (0)

    D3D12Renderer::D3D12Renderer() : m_currentBackBuffer(0), m_windowWidth(0), m_windowHeight(0)
    {
       // m_cameraSpeed = 4.0f; //World units per second
       // m_cameraInput = {0.0f, 0.0f, 0.0f};

        m_activeCameraEntity = NullEntity;
    }


    D3D12Renderer::~D3D12Renderer()
    {
    }

    bool D3D12Renderer::Init(HWND hWnd)
    {
        bool retVal = false;

        //get the client window area size.
        RECT clientSize;;
        GetClientRect(hWnd, &clientSize);
        m_windowWidth = clientSize.right;
        m_windowHeight = clientSize.bottom;

        //Define a default ViewProjMatrix.
        InitDefaultCameraMatrices();

        do {
            if (!CreateDeviceAndSwapChain(hWnd))
            {
                break;
            }
            if (!CreateDepthStencilBuffer())
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
            if (!CreateConstantBuffer())
            {
                break;
            }

            retVal = true;
        } while (0);

        return retVal;
    }

    bool D3D12Renderer::SetupRenderTargets()
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


    bool D3D12Renderer::SetupSwapChain()
    {
        // This is the first fence value we'll set, has to be != our initial value
        // below so we can wait on the first fence correctly
        m_currentBackBuffer = 0;
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

    bool D3D12Renderer::CreateDeviceAndSwapChain(HWND hWnd)
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

        // Get the DXGI factory used to create the swap chain.
        // REMARK: We are creating the factory before the device, because ideally
        // you'd want to enumerate adapters before creating a device.
        ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
        HRESULT hr = CreateDXGIFactory2(
#ifdef _DEBUG
            DXGI_CREATE_FACTORY_DEBUG,
#else
            0,
#endif
            IID_PPV_ARGS(&dxgiFactory));
        if (FAILED(hr))
        {
            ODERROR("DXGI factory creation failed.");
            return false;
        }


        //This example shows calling D3D12CreateDevice to create the device.
        //GAL: Minimum supported d3d version is 11. 
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device));

        D3D12_FEATURE_DATA_D3D12_OPTIONS options;
        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options),
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
        hr = m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(hr)) {
            ODERROR("Command queue creation failed.");
            return false;
        }

        //Create the swap chain similarly to how it was done in Direct3D 11.

        // Create the swap chain descriptor.
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = D3D12Renderer::kBackBufferCount;
        swapChainDesc.BufferDesc.Format = MY_RENDER_TARGET_FORMAT;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.SampleDesc.Count = MY_SAMPLE_DESC_COUNT;
        swapChainDesc.SampleDesc.Quality = MY_SAMPLE_DESC_QUALITY;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.Flags = 0; //DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        //Documentation recommends flip_sequential for D3D12, see - https://msdn.microsoft.com/en-us/library/windows/desktop/dn903945
        //swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL


        // Create the swap chain using the command queue, NOT using the device.  Thanks Killeak!
        hr = dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, (IDXGISwapChain**)m_swapChain.GetAddressOf());

        //increase max frame latency when required
        if (swapChainDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            m_swapChain->SetMaximumFrameLatency(D3D12Renderer::kBackBufferCount);
        }

        m_renderTargetViewDescriptorSize =
            m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        return SetupSwapChain();
    }

    bool D3D12Renderer::CreateDepthStencilBuffer()
    {
        // create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        HRESULT hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_depthStencilDescriptorHeap));
        if (FAILED(hr))
        {
            ODERROR("Failed to create DepthStencil descriptor heap. hr=0x%X", hr);
            return false;
        }
        m_depthStencilDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");


        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        hr = m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_windowWidth, m_windowHeight,
                                          1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_depthStencilBuffer)
        );
        if (FAILED(hr))
        {
            ODERROR("Failed to create DepthStencil buffer. hr=0x%X", hr);
            return false;
        }

        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc,
            m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        return true;
    }

    bool D3D12Renderer::CreateAllocatorsAndCommandLists()
    {
        for (int i = 0; i < kBackBufferCount; ++i) {
            m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_commandAllocators[i]));
            m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_commandAllocators[i].Get(), nullptr,
                IID_PPV_ARGS(&m_commandLists[i]));
            m_commandLists[i]->Close();
        }
        return true;
    }

   void D3D12Renderer::DefineViewportScissor()
    {
        m_rectScissor = { 0, 0,
            static_cast<LONG>(m_windowWidth),
            static_cast<LONG>(m_windowHeight) };

        m_viewport = { 0.0f, 0.0f,
            static_cast<float>(m_windowWidth),
            static_cast<float>(m_windowHeight),
            0.0f, 1.0f
        };
    }

   bool D3D12Renderer::CreateRootSignature()
   {
       // create a root descriptor, which explains where to find the data for this root parameter
       D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
       rootCBVDescriptor.ShaderRegister = 0; //b0 register.
       rootCBVDescriptor.RegisterSpace = 0; //Default register space.

       // create the root parameter and fill it out as a Root Descriptor (One level of indirection).
       D3D12_ROOT_PARAMETER  rootParameters[1]; // only one parameter right now
       rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
       rootParameters[0].Descriptor = rootCBVDescriptor; // this is the root descriptor for this root parameter
       rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now

       ComPtr<ID3DBlob> rootBlob;
       ComPtr<ID3DBlob> errorBlob;

       // We don't use another descriptor heap for the sampler, instead we use a
       // static sampler
       CD3DX12_STATIC_SAMPLER_DESC samplers[1];
       samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

       CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;

       // Create the root signature
       descRootSignature.Init(1, rootParameters,
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

   bool D3D12Renderer::CreatePipelineStateObject()
   {
       //Create vertex and pixel shader
       m_vertexShader = std::make_unique<Shader>(L"Shaders\\P3F_C4F_Shaders.hlsl", "VSMain", Shader::TargetType::VertexShader);
       if (m_vertexShader->GetCompileError()) {
           ODERROR("Failed to compile vertex shader: %s", m_vertexShader->GetCompileError());
           return false;
       }

       m_pixelShader = std::make_unique<Shader>(L"Shaders\\P3F_C4F_Shaders.hlsl", "PSMain", Shader::TargetType::PixelShader);
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
       psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
       psoDesc.NumRenderTargets = 1; // we only have one format in RTVFormats[8]
       psoDesc.SampleDesc.Count = MY_SAMPLE_DESC_COUNT; // must be the same sample description as the swapchain and depth/stencil buffer
       psoDesc.SampleDesc.Quality = MY_SAMPLE_DESC_QUALITY;
       psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
       psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done

       // create the pso
       HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject));
       if (FAILED(hr))
       {
           ODERROR("Failed to create PSO: result=0x%08X", hr);
           return false;
       }

       return true;
   }

   bool D3D12Renderer::CreateConstantBuffer()
   {
       return m_constantBufferPool.Init(m_device.Get());
   }

   void D3D12Renderer::UpdateCameraAndProjectionMatrices(float deltaTimeMillis)
   {
#if 0
       // build projection and view matrix
       XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14159f / 180.0f), (float)m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);
       XMStoreFloat4x4(&m_cameraProjMat, tmpMat);

       // build view matrix
       XMVECTOR position = XMLoadFloat3(&m_cameraPosition);
       //XMVECTOR velocity = XMLoadFloat3(&m_cameraInput);

       // Calculate how much to move in the current camera direction.
       XMVECTOR camForward = XMLoadFloat3(&m_cameraForward);
       XMVECTOR forwardDisplacement = camForward * m_cameraInput.z * m_cameraSpeed * deltaTimeMillis * 0.001f;

       XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
       XMVECTOR camRight = XMVector3Cross(camUp, camForward);
       camRight = XMVector3Normalize(camRight);
       XMVECTOR rightDisplacement = camRight * m_cameraInput.x * m_cameraSpeed * deltaTimeMillis * 0.001f;

       camUp = XMVector3Cross(camForward, camRight);
       camUp = XMVector3Normalize(camUp);
       XMVECTOR upDisplacement = camUp * m_cameraInput.y * m_cameraSpeed * deltaTimeMillis * 0.001f;

       //Move the camera.
       position += forwardDisplacement;
       position += rightDisplacement;
       position += upDisplacement;

       //Save the new position
       XMStoreFloat3(&m_cameraPosition, position);

       XMVECTOR cTarg = position + camForward;
       tmpMat = XMMatrixLookAtLH(position, cTarg, camUp);
       XMStoreFloat4x4(&m_cameraViewMat, tmpMat);
#endif
   }

   void D3D12Renderer::CleanUp()
   {
       // Drain the queue, wait for everything to finish
       for (int i = 0; i < kBackBufferCount; ++i) {
           WaitForFence(m_frameFences[i].Get(), m_fenceValues[i],m_frameFenceEvents[i]);
       }

       //Release the fence events
       for (auto evt : m_frameFenceEvents) {
           CloseHandle(evt);
       }

       //Release the entities and components.
       m_registry.reset();
       //Release the resources,
       m_gpuResourceManager.Clear();
   }

   void D3D12Renderer::WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent)
   {
       if (fence->GetCompletedValue() < completionValue) {
           fence->SetEventOnCompletion(completionValue, waitEvent);
           WaitForSingleObject(waitEvent, INFINITE);
       }
   }

   RenderEntityId D3D12Renderer::AddRenderNode(ResourceHandle gpuMeshHandle, const XMFLOAT4X4A& m_worldMatrix)
   {
       RenderEntityId renderEntity = m_registry.create();

       RenderNode& renderNode = m_registry.assign<RenderNode>(renderEntity);
       renderNode.m_gpuMeshHandle = gpuMeshHandle;
       renderNode.m_worldMatrix = m_worldMatrix;

       return renderEntity;
   }

   void D3D12Renderer::InitDefaultCameraMatrices()
   {
       m_cameraProjMat = MakeProjMatrix(45.0f, 0.1f, 1000.0f);
       m_cameraViewProjMat = MakeViewProjMatrix(m_cameraProjMat,
           XMVectorSet(0.0f, 0.0f, -4.0f, 1.0f),
           XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
           XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
   }

   RenderEntityId D3D12Renderer::AddCamera(float fieldOfViewDegrees, float nearClipDistance, float farClipDistance,
       XMVECTOR position, XMVECTOR forward, XMVECTOR up)
   {
       RenderEntityId camEntity = m_registry.create();
       Camera& camData = m_registry.assign<Camera>(camEntity);

       //The latest added camera becomes the current camera
       m_cameraProjMat = MakeProjMatrix(fieldOfViewDegrees, nearClipDistance, farClipDistance);
       m_cameraViewProjMat = MakeViewProjMatrix(m_cameraProjMat, position, forward, up);

       XMStoreFloat4x4A(&camData.m_projMatrix, m_cameraProjMat);
       XMStoreFloat4x4A(&camData.m_viewProjMatrix, m_cameraViewProjMat);
       m_activeCameraEntity = camEntity;

       return camEntity;
   }

   void D3D12Renderer::SetActiveCamera(RenderEntityId camId)
   {
       assert(camId != NullEntity);

       if (camId == m_activeCameraEntity)
           return; //All is good.

       Camera& camData = m_registry.get<Camera>(camId);
       m_cameraProjMat = XMLoadFloat4x4A(&camData.m_projMatrix);
       m_cameraViewProjMat = XMLoadFloat4x4A(&camData.m_viewProjMatrix);

       m_activeCameraEntity = camId;
   }

   void D3D12Renderer::UpdateCamera(RenderEntityId camId, XMVECTOR position, XMVECTOR forward, XMVECTOR up)
   {
       if (camId == NullEntity)
       {
           m_cameraViewProjMat = MakeViewProjMatrix(m_cameraProjMat, position, forward, up);
           return;
       }

       if (camId == m_activeCameraEntity)
       {
           m_cameraViewProjMat = MakeViewProjMatrix(m_cameraProjMat, position, forward, up);
           Camera& camData = m_registry.get<Camera>(camId);
           XMStoreFloat4x4A(&camData.m_viewProjMatrix, m_cameraViewProjMat);
           return;
       }

       Camera& camData = m_registry.get<Camera>(camId);
       XMMATRIX camProjMat = XMLoadFloat4x4A(&camData.m_projMatrix);
       XMMATRIX camViewProjMat = MakeViewProjMatrix(camProjMat, position, forward, up);
       XMStoreFloat4x4A(&camData.m_viewProjMatrix, camViewProjMat);
   }
    
   /////////////////////    IInputListener Start
#if 0
   void D3D12Renderer::OnMoveForward(float value, bool isDiscrete)
   {
       if (!isDiscrete)
       {
           m_cameraInput.z = value;
           return;
       }

       XMVECTOR camForward = XMLoadFloat3(&m_cameraForward);
       XMVECTOR position = XMLoadFloat3(&m_cameraPosition);
       position += camForward * value;
       XMStoreFloat3(&m_cameraPosition, position);
   }

   void D3D12Renderer::OnMoveRight(float value, bool isDiscrete)
   {
       if (!isDiscrete)
       {
           m_cameraInput.x = value;
           return;
       }

       XMVECTOR camForward = XMLoadFloat3(&m_cameraForward);
       XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

       XMVECTOR camRight = XMVector3Cross(camUp, camForward);
       camRight = XMVector3Normalize(camRight);

       XMVECTOR position = XMLoadFloat3(&m_cameraPosition);
       position += camRight * value;
       XMStoreFloat3(&m_cameraPosition, position);
   }
   
   void D3D12Renderer::OnMoveUp(float value, bool isDiscrete)
   {
       if (!isDiscrete)
       {
           m_cameraInput.y = value;
           return;
       }

       XMVECTOR camForward = XMLoadFloat3(&m_cameraForward);
       XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

       XMVECTOR camRight = XMVector3Cross(camUp, camForward);
       camRight = XMVector3Normalize(camRight);

       camUp = XMVector3Cross(camForward, camRight);
       camUp = XMVector3Normalize(camUp);

       XMVECTOR position = XMLoadFloat3(&m_cameraPosition);
       position += camUp * value;
       XMStoreFloat3(&m_cameraPosition, position);
   }

   void D3D12Renderer::OnMoveYawPitch(float yaw, float pitch)
   {
       // Calculate how much to move in the current camera direction.
       XMVECTOR camForward = XMLoadFloat3(&m_cameraForward);
       XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
       
       XMVECTOR camRight = XMVector3Cross(camUp, camForward);
       camRight = XMVector3Normalize(camRight);
       
       camUp = XMVector3Cross(camForward, camRight);
       camUp = XMVector3Normalize(camUp);

       float yawAngle = yaw * 3.14159f;
       XMMATRIX matYaw = XMMatrixRotationAxis(camUp, yawAngle);

       float pitchAngle = pitch * 3.14159f;
       XMMATRIX matPitch = XMMatrixRotationAxis(camRight, pitchAngle);

       XMMATRIX finalMat = matYaw * matPitch;

       XMVECTOR newForward = XMVector3Transform(camForward,
           finalMat);
    
       //Save the new forward vector.
       XMStoreFloat3(&m_cameraForward, newForward);
   }
#endif

   /////////////////////    IInputListener End


   void D3D12Renderer::PreRender()
   {
       m_commandAllocators[m_currentBackBuffer]->Reset();

       auto commandList = m_commandLists[m_currentBackBuffer].Get();
       commandList->Reset(
           m_commandAllocators[m_currentBackBuffer].Get(), nullptr);

       D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
       CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(renderTargetHandle,
           m_renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
           m_currentBackBuffer, m_renderTargetViewDescriptorSize);

       CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilHandle(m_depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

       commandList->OMSetRenderTargets(1, &renderTargetHandle, true, &depthStencilHandle);
       commandList->RSSetViewports(1, &m_viewport);
       commandList->RSSetScissorRects(1, &m_rectScissor);

       // Transition back buffer
       D3D12_RESOURCE_BARRIER barrier;
       barrier.Transition.pResource = m_renderTargets[m_currentBackBuffer].Get();
       barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
       barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
       barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
       barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
       barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

       commandList->ResourceBarrier(1, &barrier);

       static const float clearColor[] = {
           0.0f, 0.0f, 0.0f,
           1.0f
       };

       commandList->ClearRenderTargetView(renderTargetHandle,
           clearColor, 0, nullptr);
       commandList->ClearDepthStencilView(depthStencilHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

   } // RendererD3D12::PrepareRender()

   void D3D12Renderer::PostRender()
   {
       // Transition the swap chain back to present
       D3D12_RESOURCE_BARRIER barrier;
       barrier.Transition.pResource = m_renderTargets[m_currentBackBuffer].Get();
       barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
       barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
       barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
       barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
       barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

       auto commandList = m_commandLists[m_currentBackBuffer].Get();
       commandList->ResourceBarrier(1, &barrier);

       commandList->Close();

       // Execute our commands
       ID3D12CommandList* commandLists[] = { commandList };
       m_commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);

   } //RendererD3D12::FinalizeRender()

   void D3D12Renderer::CalculateWVPMatrixForShader(XMFLOAT4X4& wvpMatrixOut, const XMFLOAT4X4& objWorldMatrix)
   {
       XMMATRIX objWorldMat = XMLoadFloat4x4(&objWorldMatrix);
       XMMATRIX wvpMat = objWorldMat * m_cameraViewProjMat;

       //Remember that HLSL is column major, so we need to transpose.
       XMStoreFloat4x4(&wvpMatrixOut, XMMatrixTranspose(wvpMat));
   }

   ///////////////////////////////////////////////////////////////////////////////
   /**
    * Present the current frame by swapping the back buffer, then move to the
    * next back buffer and also signal the fence for the current queue slot entry.
    */
   void D3D12Renderer::Swap()
   {
       m_swapChain->Present(0, 0);

       // Mark the fence for the current frame.
       const auto fenceValue = m_currentFenceValue;
       m_commandQueue->Signal(m_frameFences[m_currentBackBuffer].Get(), fenceValue);
       m_fenceValues[m_currentBackBuffer] = fenceValue;
       ++m_currentFenceValue;

       // Take the next back buffer from our chain
       m_currentBackBuffer = (m_currentBackBuffer+ 1) % kBackBufferCount;

   } //RendererD3D12::Swap()

    void D3D12Renderer::RenderFrame(float deltaTimeMillis)
    {
        WaitForFence(m_frameFences[m_currentBackBuffer].Get(),
            m_fenceValues[m_currentBackBuffer],
            m_frameFenceEvents[m_currentBackBuffer]);

        UpdateCameraAndProjectionMatrices(deltaTimeMillis);

        PreRender();

        auto commandList = m_commandLists[m_currentBackBuffer].Get();
        //One day
        //UpdateConstantBuffer();

        // Set our state (shaders, etc.)
        commandList->SetPipelineState(m_pipelineStateObject.Get());

        // Set our root signature
        commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        // Set descriptor heaps....
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        GpuMeshCache& gpuMeshCache = m_gpuResourceManager.GetGpuMeshCache();
        int renderObjectIdx = 0;
        auto rawView = m_registry.raw_view<RenderNode>();
        for (auto& renderNode : rawView)
        {
            //Update
            PerObjectConstantBufferData cbData;
            CalculateWVPMatrixForShader(cbData.wvpMatrix, renderNode.m_worldMatrix);

            D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBufferPool.UploadData(&cbData, m_currentBackBuffer, renderObjectIdx);
            commandList->SetGraphicsRootConstantBufferView(0, cbvGpuAddress);

            //Get the gpuMeshResource.
            std::shared_ptr<GpuMesh> gpuMesh = gpuMeshCache.GetResource(renderNode.m_gpuMeshHandle);
            commandList->IASetVertexBuffers(0, 1, gpuMesh->GetVertexBufferView());
            commandList->IASetIndexBuffer(gpuMesh->GetIndexBufferView());
            commandList->DrawIndexedInstanced(gpuMesh->GetNumIndices(), 1, 0, 0, 0);

            renderObjectIdx++;
        }

        PostRender();
        Swap();

    } // RendererD3D12::RenderFrame()

}; //namespace GAL