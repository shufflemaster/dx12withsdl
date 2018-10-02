#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

namespace GAL
{
    class RendererD3D12
    {
    public:
        static  RendererD3D12& GetRenderer() { return s_renderer; }
        virtual ~RendererD3D12();

        bool Init(HWND hWnd);
        void RenderFrame();
        HRESULT ResizeSwapChain();

        ID3D12Device* GetDevice() { return mDevice.Get();  }
        ID3D12GraphicsCommandList* GetCommandList() { return mCommandList.Get(); }

        //FIXME: This is more related with the Technique/Effect.
        void fillPSOSampleDescription(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDescription);

    private:
        RendererD3D12();
        static  RendererD3D12 s_renderer;

        static const UINT kBackBufferCount = 3; //define number of backbuffers to use

        //Rule of thumbs
        /*
        Render Targets : Number of frame buffers
        Command Allocators : Number of frame buffers * number of threads
        Command Lists : Number of threads.  NOTE: Command Allocators can only have one command list recording at any time
        Fences : Number of threads
        Fence Values : Number of threads
        Fence Events : Number of threads
        */

        Microsoft::WRL::ComPtr<ID3D12Device> mDevice;					//d3d12 device
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandListAllocator; //d3d12 command list allocator
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue; //d3d12 command queue
        Microsoft::WRL::ComPtr<IDXGIDevice2> mDXGIDevice; //DXGI device
        Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;   // the pointer to the swap chain interface
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;  //d3d12 command list
        Microsoft::WRL::ComPtr<ID3D12Fence> mFence; //fence used by GPU to signal when command queue execution has finished
        Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTarget[kBackBufferCount]; //backbuffer resource, like d3d11's ID3D11Texture2D, array of 2 for flip_sequential support
        //CDescriptorHeapWrapper mRTVDescriptorHeap; //descriptor heap wrapper class instance, for managing RTV descriptor heap
    };
}

