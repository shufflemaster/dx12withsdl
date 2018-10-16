// DX12withSDL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// include the basic windows header files and the Direct3D header files
//windows.h, d3d12.h, DirectXMath.h, D3Dcompiler.h, dxgi.h

#include "pch.h"
#include <windows.h>
#include <windowsx.h>


#include <SDL.h>
#undef main

//helper class/functions for D3D12 taken from documentation pages
#include "PerfCounter.h"
#include "RendererD3D12.h"
#include "VertexTypes.h"
#include "RenderNode.h"

// global declarations
HWND g_hWnd;

static void LoadScene(GAL::RendererD3D12& renderer)
{
#define SIZEF 0.5f
    //Screen Coordinates are left, bottom = -1.0, -1.0.  right, top =1.0, 1.0
    const GAL::P3F_C4F vertices[] = {
        //pos                 , //color
      { { 0.0f,  SIZEF, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
      { { SIZEF, -SIZEF, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
      { {-SIZEF, -SIZEF, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };

    const DWORD indices[] = { 0, 1, 2 };

    GAL::RenderNode* node = new GAL::RenderNode();
    node->InitWithVertices(vertices, sizeof(vertices)/sizeof(vertices[0]),
        indices, sizeof(indices)/sizeof(indices[0]));

    renderer.AddRenderNode(node);
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

    int frameCnt = 0;
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
        if (frameCnt == 500) {
            break;
        }
    }

    perfCounter.stop();
    //FILE* pFile = fopen("test.txt", "w");
    perfCounter.dump(frameCnt);// , pFile);
    double seconds = perfCounter.getElapsedUS() / 1000000.0;
    GAL::odprintf("FPS=%.2f\n", (double)frameCnt/seconds);
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
