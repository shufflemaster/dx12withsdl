#pragma once

using namespace std;
using namespace DirectX;

#include "ConstantBufferPool.hpp"
#include "d3d12renderer/GpuResourceManager.h"

using Microsoft::WRL::ComPtr;

namespace GAL
{
    class Shader;

    using RenderEntityId = Registry::entity_type;

    struct PerObjectConstantBufferData {
        XMFLOAT4X4 wvpMatrix;
    };

    //FIXME! There's no reason for the Renderer to be an input listener
    class D3D12Renderer
    {
    public:
        D3D12Renderer();
        virtual ~D3D12Renderer();

        bool Init(HWND hWnd);
        void RenderFrame(float deltaTimeMillis);
        void CleanUp();

        ID3D12Device* GetDevice() { return m_device.Get();  }
        ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
        static void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent);
        GpuResourceManager& GetGpuResourceManager() { return m_gpuResourceManager; }

        //The idea is that the Renderer returns a handle to a private-to-the-renderer entity
        //So if the engine decides to destroy an Entity it can also use this handle to notify
        //the renderer to also distroy its renderer counter part.
        RenderEntityId AddRenderNode(ResourceHandle gpuMeshHandle, const XMFLOAT4X4A& m_worldMatrix);

        void InitDefaultCameraMatrices();

        inline XMMATRIX MakeProjMatrix(float fieldOfViewDegrees, float nearClipDistance, float farClipDistance) const
        {
            assert((m_windowWidth != 0) && (m_windowHeight != 0));
            return XMMatrixPerspectiveFovLH(fieldOfViewDegrees*(3.14159f / 180.0f), (float)m_windowWidth / (float)m_windowHeight, nearClipDistance, farClipDistance);
        }

        inline XMMATRIX MakeViewProjMatrix(XMMATRIX projMatrix, XMVECTOR position, XMVECTOR forward, XMVECTOR up) const
        {
            XMMATRIX viewMat = XMMatrixLookToLH(position, forward, up);
            return viewMat * projMatrix;
        }

        //Returns the instance that owns the camera.
        RenderEntityId AddCamera(float fieldOfViewDegrees, float nearClipDistance, float farClipDistance,
            XMVECTOR position, XMVECTOR forward, XMVECTOR up);
        void SetActiveCamera(RenderEntityId camId);
        void UpdateCamera(RenderEntityId camId, XMVECTOR position, XMVECTOR forward, XMVECTOR up);

    private:
        bool SetupRenderTargets();
        bool SetupSwapChain();
        bool CreateDeviceAndSwapChain(HWND hWnd); //Init
        bool CreateDepthStencilBuffer(); //Init
        bool CreateAllocatorsAndCommandLists(); //Init
        void DefineViewportScissor(); //Init
        bool CreateRootSignature(); //Init
        bool CreatePipelineStateObject(); //Init
        bool CreateConstantBuffer(); //Init

        void PreRender();
        void PostRender();
        void Swap();

        void CalculateWVPMatrixForShader(XMFLOAT4X4& wvpMatrixOut, const XMFLOAT4X4& objWorldMatrix);

        static const UINT kBackBufferCount = 3; //define number of backbuffers to use

        //Rule of thumbs
        /*
        Render Targets : Number of frame buffers
        Command Queue: 1 for rendering, 1 for compute, 1 for copy. So we use 1.
        Command Allocators : Number of frame buffers * number of threads
        Command Lists : Number of frame buffers * number of threads. Number of threads.  NOTE: Command Allocators can only have one command list recording at any time
        Fences : Number of threads
        Fence Values : Number of threads
        Fence Events : Number of threads
        */

        ComPtr<ID3D12Device> m_device;					//d3d12 device
        ComPtr<ID3D12CommandQueue> m_commandQueue; //d3d12 command queue
        ComPtr<IDXGISwapChain3> m_swapChain;   // the pointer to the swap chain interface

        ComPtr<ID3D12DescriptorHeap> m_renderTargetDescriptorHeap;
        ComPtr<ID3D12Resource> m_renderTargets[kBackBufferCount]; //backbuffer resource, like d3d11's ID3D11Texture2D, array of 2 for flip_sequential support

        ComPtr<ID3D12CommandAllocator> m_commandAllocators[kBackBufferCount]; //d3d12 command list allocators
        ComPtr<ID3D12GraphicsCommandList> m_commandLists[kBackBufferCount] ;  //d3d12 command list for the renderer. All other threads will have its own command list.
        
        ComPtr<ID3D12DescriptorHeap> m_depthStencilDescriptorHeap;
        ComPtr<ID3D12Resource> m_depthStencilBuffer;

        //As a rendering pass, we expect all objects to have a World matrix.
        //The renderer owns the View and Projection matrices. And the renderer
        //will take each object world matrix and generate a unique W*V*P matrix that will
        //be fed to b0 shader register.
        //This the pool of constant buffers that will hold the WVP Matrix for each visible object in the scene.
        ConstantBufferPool<PerObjectConstantBufferData, kBackBufferCount, 1024> m_constantBufferPool; 

        //The fences
        HANDLE m_frameFenceEvents[kBackBufferCount];
        ComPtr<ID3D12Fence> m_frameFences[kBackBufferCount];
        UINT64 m_fenceValues[kBackBufferCount];

        UINT64 m_currentFenceValue;
        int32_t m_currentBackBuffer; //A value between 0 to kBackBufferCount - 1. Changes per frame within that range.
        int32_t m_renderTargetViewDescriptorSize;

        UINT m_windowWidth, m_windowHeight;
        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_rectScissor;

        XMMATRIX m_cameraProjMat; //Contains proj matrix;
        XMMATRIX m_cameraViewProjMat; //Contains View and Projection in one matrix.
        RenderEntityId m_activeCameraEntity;
#if 0
        float m_cameraSpeed;
        XMFLOAT3 m_cameraInput;
        XMFLOAT3 m_cameraForward;
        XMFLOAT3 m_cameraPosition;
#endif


        ComPtr <ID3D12RootSignature> m_rootSignature; // root signature defines data shaders will access
        ComPtr <ID3D12PipelineState> m_pipelineStateObject; // pso containing a pipeline state (shaders are also part of the state).
        //FIXME: Shader should not be here. Better create a Material class that has a shaders.
        unique_ptr<Shader> m_vertexShader;
        unique_ptr<Shader> m_pixelShader;

        GpuResourceManager m_gpuResourceManager;
        Registry m_registry; //The database of render nodes.
    };
}

