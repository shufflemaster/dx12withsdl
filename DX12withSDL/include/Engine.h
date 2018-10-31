#pragma once

namespace GAL
{
    class Universe;
    class ResourceManager;
    class InputSystem;

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
        ResourceManager m_resourceManager;

        //The Systems One by One (Created with help of Universe).
        InputSystem* m_inputSystem;

        //returns true on success.
        //Given a level name Loads all entities and their components.
        //in levelName is null, loads a hard coded level.
        bool LoadLevel(const char *levelName = nullptr);

        bool CreateSystems(HWND hWnd);
    };

}; //namespace GAL
