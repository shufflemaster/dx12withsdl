#pragma once

namespace GAL
{
    class Universe;
    class ResourceManager;

    class Engine
    {
    public:
        Engine();
        ~Engine();

        //Blocking call, When it returns the game/simulation is finished.
        //returns 0 on success (no early termination).
        int Run(HWND mainWindow);

    private:
        Universe m_universe;
        ResourceManager m_resourceManager;

        //returns true on success.
        //Given a level name Loads all entities and their components.
        //in levelName is null, loads a hard coded level.
        bool LoadLevel(const char *levelName = nullptr);
    };

}; //namespace GAL
