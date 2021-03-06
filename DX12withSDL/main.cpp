// DX12withSDL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// include the basic windows header files and the Direct3D header files
//windows.h, d3d12.h, DirectXMath.h, D3Dcompiler.h, dxgi.h

#include "pch.h"
#undef main

#include "Engine.h"

int main(int argc, char *args[]) {
    SDL_Window* window = SDL_CreateWindow("DirectX 12 Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    HWND hWnd = GetActiveWindow();
    GAL::Engine engine;

    int retVal = engine.Run(hWnd);

    SDL_DestroyWindow(window);

    return retVal;
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
