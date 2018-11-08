#include "pch.h"

#include "PerfCounter.h"
#include "TimeCounter.h"
#include "VertexTypes.h"
#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"
#include "HashedString.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "Universe.h"

#include "Systems/InputSystem.h"
#include "Systems/RenderSystem.h"
#include "ResourceTypes/Mesh.h"
#include "ResourceLoaders/TriangleMeshLoader.h"

using namespace DirectX;

//helper for LoadLevel
static inline float randFloat(float min, float max)
{
    int random_number = std::rand();
    float normalized = (float)random_number / (float)RAND_MAX;
    float retVal = (max - min)*normalized + min;
    return retVal;
}

//helper for LoadLevel
static void GenerateRandomWorldMatrix(XMFLOAT4X4& worldMatOut,
    float minX, float maxX,
    float minY, float maxY,
    float minZ, float maxZ)
{
    XMMATRIX tmpMat = XMMatrixTranslation(randFloat(minX, maxX), randFloat(minY, maxY), randFloat(minZ, maxZ));
    XMStoreFloat4x4(&worldMatOut, tmpMat);
}

namespace GAL
{
    Engine::Engine()
    {
    }


    Engine::~Engine()
    {
    }

    int Engine::Run(HWND hWnd)
    {
        if (!m_renderer.Init(hWnd)) {
            ODERROR("Failed to initialize the renderer");
            return -1;
        }

        if (!LoadLevel())
        {
            return -1;
        }

        if (!CreateAndInitSystems(hWnd))
        {
            return -1;
        }

        float deltaTimeMilis = 0.0f;
        unsigned int frameCnt = 0;
        GAL::PerfCounter perfCounter;
        GAL::TimeCounter timerCounter;

        perfCounter.start();
        timerCounter.start();

        while (TRUE)
        {
            SDL_Event windowEvent;
            if (SDL_PollEvent(&windowEvent)) {
                if (windowEvent.type == SDL_QUIT) break;
                m_inputSystem->ProcessEvent(windowEvent);
            }

            deltaTimeMilis = (float)timerCounter.readDeltaTimeMillis();
            //GAL::odprintf("deltaTimeMilis=%f", deltaTimeMilis);

            m_universe.UpdateSystems(deltaTimeMilis);
            m_renderer.RenderFrame(deltaTimeMilis);

            ++frameCnt;
        }

        m_renderer.CleanUp();

        perfCounter.stop();
        //FILE* pFile = fopen("test.txt", "w");
        perfCounter.dump(frameCnt);// , pFile);
        double seconds = perfCounter.getElapsedSeconds();
        GAL::odprintf("frameCnt=%u FPS=%.2f\n", frameCnt, (double)frameCnt / seconds);
        //fclose(pFile);
        return 0;
    }

    bool Engine::LoadLevel(const char *levelName)
    {
        //We don't support loading levels by name, yet.
        assert(levelName == nullptr);

        //srand((unsigned int)time(NULL));
        srand(666);


        MeshCache& meshCache = m_universe.GetResourceManager().GetMeshCache();
        Registry& registry = m_universe.GetRegistry();

        //Create all the triangle entities.
        const int kMaxTriangles = 1000;
        std::stringstream meshResourceNameStream;
        std::string meshResourceName;
        for (int i = 0; i < kMaxTriangles; i++)
        {
            auto newEntity = m_universe.CreteEntity();

            //Create the mesh component
            meshResourceNameStream << "assets/mesh/triangle" << i;
            MeshComponent& meshComponent = registry.assign<MeshComponent>(newEntity);
            meshComponent.m_filename = meshResourceNameStream.str();
            meshResourceNameStream.str(std::string());

            float rndRed = randFloat(0.2f, 1.0f);
            float rndGreen = randFloat(0.2f, 1.0f);
            float rndBlue = randFloat(0.2f, 1.0f);
            meshComponent.m_meshHandle = meshCache.AddResource<TriangleMeshLoader>(meshComponent.m_filename, 0.5f, rndRed, rndGreen, rndBlue);
            if (meshComponent.m_meshHandle < 0)
            {
                ODERROR("Failed to create mesh for triangle %d", i);
                return false;
            }

            //Add the transform component
            TransformComponent& transformComponent = registry.assign<TransformComponent>(newEntity);
            ::GenerateRandomWorldMatrix(transformComponent.m_matrix, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 10.0f);
        }

        //Create the camera entity



        //GAL::RenderNode* node = new GAL::RenderNode();
        //renderer.AddRenderNode(node);

        return true;
    }

    bool Engine::CreateAndInitSystems(HWND hWnd)
    {
        //The order in which we add it here determines the order of Tick Update.
        m_inputSystem = static_cast<InputSystem*>(m_universe.CreateAndAddSystem<InputSystem>(hWnd));
        m_renderSystem = static_cast<RenderSystem*>(m_universe.CreateAndAddSystem<RenderSystem>(&m_renderer));

        m_universe.InitializeSystems();

        return true;
    }

} //namespace GAL