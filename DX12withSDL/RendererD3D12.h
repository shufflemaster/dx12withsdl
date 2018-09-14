#pragma once

namespace GAL
{
    class RendererD3D12
    {
    public:
        RendererD3D12();
        virtual ~RendererD3D12();

        bool Init(HWND hWnd);
        void RenderFrame();

    private:
        const UINT kBackBufferCount = 4; //define number of backbuffers to use
        Microsoft::WRL::ComPtr<ID3D12Device> mDevice;					//d3d12 device
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandListAllocator; //d3d12 command list allocator
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue; //d3d12 command queue
        Microsoft::WRL::ComPtr<IDXGIDevice2> mDXGIDevice; //DXGI device
        Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;   // the pointer to the swap chain interface
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;  //d3d12 command list
        Microsoft::WRL::ComPtr<ID3D12Fence> mFence; //fence used by GPU to signal when command queue execution has finished
        Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTarget[g_bbCount]; //backbuffer resource, like d3d11's ID3D11Texture2D, array of 2 for flip_sequential support
        CDescriptorHeapWrapper mRTVDescriptorHeap; //descriptor heap wrapper class instance, for managing RTV descriptor heap

        HRESULT ResizeSwapChain();
    };
}

