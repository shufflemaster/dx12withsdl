#pragma once


#include <stdint.h>
#include <vector>
#include <memory>
using namespace std;

#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace GAL
{
    class Shader;
    class RenderNode;

    class RendererD3D12
    {
    public:
        static  RendererD3D12& GetRenderer() { return s_renderer; }
        virtual ~RendererD3D12();

        bool Init(HWND hWnd);
        void RenderFrame();
        void AddRenderNode(RenderNode* node);
        void CleanUp();

        ID3D12Device* GetDevice() { return m_device.Get();  }
        ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
        static void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent);

    private:
        RendererD3D12();

        bool SetupRenderTargets();
        bool SetupSwapChain();
        bool CreateDeviceAndSwapChain(HWND hWnd); //Init
        bool CreateDepthStencilBuffer(); //Init
        bool CreateAllocatorsAndCommandLists(); //Init
        void DefineViewportScissor(); //Init
        bool CreateRootSignature(); //Init
        bool CreatePipelineStateObject(); //Init

        void PreRender();
        void PostRender();
        void Swap();

        static  RendererD3D12 s_renderer;

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

        ComPtr <ID3D12RootSignature> m_rootSignature; // root signature defines data shaders will access
        ComPtr <ID3D12PipelineState> m_pipelineStateObject; // pso containing a pipeline state (shaders are also part of the state).
        //FIXME: Shader should not be here. Better create a Material class that has a shaders.
        unique_ptr<Shader> m_vertexShader;
        unique_ptr<Shader> m_pixelShader;

        vector<RenderNode*> m_renderNodes;
    };
}

