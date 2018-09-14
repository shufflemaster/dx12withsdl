// DX12withSDL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// include the basic windows header files and the Direct3D header files
//windows.h, d3d12.h, DirectXMath.h, D3Dcompiler.h, dxgi.h

#include "pch.h"
#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <vector>

//helper class/functions for D3D12 taken from documentation pages
#include "helpers.h"
#include "textureloader.h"
#include "PerfCounter.h"
#include "RendererD3D12.h"

#include <SDL.h>
#undef main


// global declarations
RendererD3D12 g_renderer;
D3D12_VIEWPORT mViewPort; //viewport, same as d3d11
RECT mRectScissor;
HANDLE mHandle; //fired by the fence when the GPU signals it, CPU waits on this event handle
Shader g_VS;
Shader g_PS;
RootSignature g_RootSig;
PipelineStateObject g_PSO;
VertexBufferResource g_VB;

//Constant buffer resources, mapped pointers, and descriptor heap for view/proj CBVs
CUploadBufferWrapper mWorldMatrix;
CUploadBufferWrapper mViewMatrix;
CUploadBufferWrapper mProjMatrix;
CDescriptorHeapWrapper mCBDescriptorHeap;

//texture support
Microsoft::WRL::ComPtr<ID3D12Resource> mTexture2D; //default heap resource, GPU will copy texture resource to these from upload buffer
CDescriptorHeapWrapper mSamplerHeap;

//Fullscreen support
HWND g_hWnd;
BOOL g_requestResize = false;





// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void CleanD3D(void);        // closes Direct3D and releases memory
void Frame();				// called once per frame to build then execute command list, and then present frame
void WaitForCommandQueueFence(); //function called by command queue after executing command list, blocks CPU thread until GPU signals mFence
HRESULT ResizeSwapChain(); //resizes the swapchain buffers to the client window size, recreates the RTVs

/*
                            // the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = "WindowClass";

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, 800, 600 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
        "WindowClass",
        "Our First D3D12 Program",
        WS_OVERLAPPEDWINDOW,
        300,
        100,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    g_hWnd = hWnd;

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    InitD3D(hWnd);

    // enter the main loop:

    MSG msg;

    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }
        else
        {
            Frame();
            // Run game code here
            // ...
            // ...
        }
    }

    // clean up DirectX and COM
    CleanD3D();

    return (int)msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    case WM_SIZE:
        {
            g_requestResize = true;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}*/


// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd)
{

    //changed shader compile target to HLSL 5.0
    g_VS.Load("Shaders.hlsl", "VSMain", "vs_5_0");
    g_PS.Load("Shaders.hlsl", "PSMain", "ps_5_0");
    //changed root sig function to include 2 root parameters: A root CBV of worldmatrix, 
    //and a two entry descriptor table for view and proj matrix CBVs
    g_RootSig.Create(mDevice.Get());


    VertexTypes::P3F_T2F triangleVerts[] =
    {
        { 0.0f, 0.5f, 0.0f, { 0.5f, 0.0f } },
        { 0.45f, -0.5, 0.0f, { 1.0f, 1.0f } },
        { -0.45f, -0.5f, 0.0f, { 0.0f, 1.0f } }
    };

    g_VB.Create(
        mDevice.Get(), sizeof(triangleVerts),
        sizeof(VertexTypes::P3F_T2F), triangleVerts);


    g_PSO.Create(
        mDevice.Get(),
        PipelineStateObjectDescription::Simple(
            VertexTypes::P3F_T2F::GetInputLayoutDesc(),
            g_RootSig,
            g_VS, g_PS
        ));

    //With the command list allocator and a PSO, you can create the actual command list, which will be executed at a later time.
    //This example shows calling ID3D12Device::CreateCommandList.
    hr = mDevice->CreateCommandList(
        1, D3D12_COMMAND_LIST_TYPE_DIRECT,
        mCommandListAllocator.Get(), g_PSO.Get(),
        __uuidof(ID3D12CommandList), (void**)&mCommandList);
    ThrowIfFailed(hr);

    //create a GPU fence that will fire an event once the command list has been executed by the command queue.
    mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&mFence);
    //And the CPU event that the fence will fire off
    mHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);


    //Define the viewport that you will be rendering to.This example shows how to set the viewport to the same size as the Win32 window client.Also note that mViewPort is a member variable.
    //Whenever a command list is reset, you must attach the view port state to the command list before the command list is executed.
    //mViewPort will let you do so without needing to redefine it every frame.

    //create a sampler descriptor heap and a valid sampler descriptor on it.
    mSamplerHeap.Create(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1, true);
    D3D12_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    mDevice->CreateSampler(&samplerDesc, mSamplerHeap.hCPU(0));

    //create an upload buffer that the CPU can copy data into 
    CUploadBufferWrapper textureUploadBuffer;
    UINT bufferSize = 1024 * 1024 * 10; //10MB
    hr = textureUploadBuffer.Create(mDevice.Get(), bufferSize, D3D12_HEAP_TYPE_UPLOAD);

    //loads dds texture file into an upload buffer then issues a command for the GPU to copy it to a default resource. 
    //See CreateD3DResources in textureloader.h for the details.
    hr = CreateTexture2D(mDevice.Get(), mCommandList.Get(), &textureUploadBuffer, L"seafloor2.dds", mTexture2D.GetAddressOf());

    // Transition the texture resource to a generic read state.
    setResourceBarrier(mCommandList.Get(), mTexture2D.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

    //The commandlist now contains the uploadbuffer-to-default-resource copy command, as well as the barrier
    //to transition the default resource to a generic read state.  Those operations must be executed before the resource
    //is ready for use in the render loop.
    mCommandList->Close();
    mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)mCommandList.GetAddressOf());

    //create an SRV descriptor of the texture in the same descriptor heap as the CBV descriptors. 
    D3D12_RESOURCE_DESC resDesc = mTexture2D->GetDesc();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = resDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = resDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    mDevice->CreateShaderResourceView(mTexture2D.Get(), &srvDesc, mCBDescriptorHeap.hCPU(2)); //index location is 2 since slots 0 and 1 contain CBVs.

    //wait for GPU to signal it has finished processing the queued command list(s).
    WaitForCommandQueueFence();

    // Command list allocators can be only be reset when the associated command lists have finished execution on the GPU; 
    // apps should use fences to determine GPU execution progress.
    hr = mCommandListAllocator->Reset();
    hr = mCommandList->Reset(mCommandListAllocator.Get(), NULL);

}

//Assigns an event to mFence, and sets the fence's completion signal value. 
//Then asks the GPU to signal that fence, and asks the CPU to wait for the event handle.
void WaitForCommandQueueFence()
{
    //reset the fence signal
    mFence->Signal(0);
    //set the event to be fired once the signal value is 1
    mFence->SetEventOnCompletion(1, mHandle);

    //after the command list has executed, tell the GPU to signal the fence
    mCommandQueue->Signal(mFence.Get(), 1);

    //wait for the event to be fired by the fence
    WaitForSingleObject(mHandle, INFINITE);
}




// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
    //ensure we're not fullscreen
    mSwapChain->SetFullscreenState(FALSE, NULL);

    //close the event handle so that mFence can actually release()
    CloseHandle(mHandle);
}

void main(int argc, char *args[]) {
    SDL_Window* window = SDL_CreateWindow("DirectX 12 Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);

    g_hWnd = GetActiveWindow();
    g_renderer.Init(g_hWnd);

    int frameCnt = 0;
    GAL::PerfCounter perfCounter;

    perfCounter.start();

    while (TRUE)
    {
        SDL_Event windowEvent;
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) break;
        }

        g_renderer.RenderFrame();

        ++frameCnt;
        if (frameCnt == 10000) {
            break;
        }
    }

    perfCounter.stop();
    //FILE* pFile = fopen("test.txt", "w");
    perfCounter.dump(frameCnt);// , pFile);
    double seconds = perfCounter.getElapsedUS() / 1000000.0;
    GAL::odprintf("FPS=%.2f\n", (double)frameCnt/seconds);
    //fclose(pFile);

    CleanD3D();
}



#if 0
#include "pch.h"
#include <iostream>

int main()
{
    std::cout << "Hello World!\n"; 
}
#endif

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
