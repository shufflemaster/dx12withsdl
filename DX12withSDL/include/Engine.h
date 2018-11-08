#pragma once

#include "Universe.h"
#include "d3d12renderer/D3D12Renderer.h"

namespace GAL
{
    class InputSystem;
    class RenderSystem;

    class Engine
    {
    public:
        Engine();
        ~Engine();

        //Blocking call, When it returns the game/simulation is finished.
        //returns 0 on success (no early termination).
        int Run(HWND hWnd);

    private:
        Universe m_universe;
        D3D12Renderer m_renderer;

        //The Systems One by One (Created with help of Universe).
        InputSystem* m_inputSystem;
        RenderSystem* m_renderSystem;

        //returns true on success.
        //Given a level name Loads all entities and their components.
        //in levelName is null, loads a hard coded level.
        bool LoadLevel(const char *levelName = nullptr);

        bool CreateAndInitSystems(HWND hWnd);
    };

}; //namespace GAL
