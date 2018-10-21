// DX12withSDL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// include the basic windows header files and the Direct3D header files
//windows.h, d3d12.h, DirectXMath.h, D3Dcompiler.h, dxgi.h

#include "pch.h"
#include <windowsx.h>

//For random number generation
#include <cstdlib>
#include <ctime>

#include <DirectXMath.h>
using namespace DirectX;

#include <SDL.h>
#undef main

//helper class/functions for D3D12 taken from documentation pages
#include "PerfCounter.h"
#include "RendererD3D12.h"
#include "VertexTypes.h"
#include "RenderNode.h"

// global declarations
HWND g_hWnd;

static inline float randFloat(float min, float max)
{
    int random_number = std::rand(); // rand() return a number between ​0​ and RAND_MAX
    float normalized = (float)random_number / (float)RAND_MAX;
    float retVal = (max - min)*normalized + min;
    return retVal;
}

//Generates traingle Model Data centered at 0,0,0
static void GenerateRandomTriangle(GAL::P3F_C4F* vertices, float minSize, float maxSize)
{
    float triangleSize = 0.5;//randFloat(minSize, maxSize);

    float rndRed = randFloat(0.2f, 1.0f);
    float rndGreen = randFloat(0.2f, 1.0f);
    float rndBlue = randFloat(0.2f, 1.0f);

    vertices[0] = { {  0.0f,          triangleSize, 0.0f}, {rndRed, rndGreen, rndBlue, 1.0f} };
    vertices[1] = { {  triangleSize, -triangleSize, 0.0f}, {rndRed, rndGreen, rndBlue, 1.0f} };
    vertices[2] = { { -triangleSize, -triangleSize, 0.0f}, {rndRed, rndGreen, rndBlue, 1.0f} };
}

static void GenerateRandomWorldMatrix(XMFLOAT4X4& worldMatOut,
    float minX, float maxX,
    float minY, float maxY,
    float minZ, float maxZ)
{
    XMMATRIX tmpMat = XMMatrixTranslation(randFloat(minX, maxX), randFloat(minY, maxY), randFloat(minZ, maxZ));
    XMStoreFloat4x4(&worldMatOut, tmpMat);
}

//Some stats: Without Depth Buffer in Razer Laptop.
//10000 triangles. FPS=267.06
// 1000 triangles. FPS=2034.21
//  500 triangles. FPS=3188.81
//  100 triangles. FPS=3822.71
//   10 triangles. FPS=3918.38 .. 4187.11
// With Depth Buffer
//10000 triangles. FPS=809.26
// 1000 triangles. FPS=3162.90
//  500 triangles. FPS=3622.99
//  100 triangles. FPS=3512.64
//   10 triangles. FPS=3662.17
static void LoadScene(GAL::RendererD3D12& renderer)
{
    //srand((unsigned int)time(NULL));
    srand(666);

    const int kMaxTriangles = 1024;

    const DWORD indices[] = { 0, 1, 2 };

    GAL::P3F_C4F tmpVertices[3];
    for (int i = 0; i < kMaxTriangles; i++)
    {
        GenerateRandomTriangle(tmpVertices, 0.1f, 0.4f);
        GAL::RenderNode* node = new GAL::RenderNode();
        node->InitWithVertices(tmpVertices, sizeof(tmpVertices) / sizeof(tmpVertices[0]),
            indices, sizeof(indices) / sizeof(indices[0]));
        GenerateRandomWorldMatrix(node->m_worldMatrix,  -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 10.0f);

        renderer.AddRenderNode(node);
    }

}

int main(int argc, char *args[]) {
    SDL_Window* window = SDL_CreateWindow("DirectX 12 Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);

    g_hWnd = GetActiveWindow();
    GAL::RendererD3D12& renderer = GAL::RendererD3D12::GetRenderer();
    if (!renderer.Init(g_hWnd)) {
        ODERROR("Failed to initialize the renderer");
        return -1;
    }

    LoadScene(renderer);

    unsigned int frameCnt = 0;
    GAL::PerfCounter perfCounter;

    perfCounter.start();

    while (TRUE)
    {
        SDL_Event windowEvent;
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) break;
        }

        renderer.RenderFrame();

        ++frameCnt;
    }

    renderer.CleanUp();

    perfCounter.stop();
    //FILE* pFile = fopen("test.txt", "w");
    perfCounter.dump(frameCnt);// , pFile);
    double seconds = perfCounter.getElapsedUS() / 1000000.0;
    GAL::odprintf("frameCnt=%u FPS=%.2f\n", frameCnt, (double)frameCnt/seconds);
    //fclose(pFile);

}


#if 0
#include "pch.h"
#include <iostream>

int main()
{
    std::cout << "Hello World!\n"; 
    return 0;
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
