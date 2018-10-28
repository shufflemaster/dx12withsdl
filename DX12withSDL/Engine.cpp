#include "pch.h"

#include "PerfCounter.h"
#include "TimeCounter.h"
#include "RendererD3D12.h"
#include "VertexTypes.h"
#include "RenderNode.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "TriangleMeshLoader.h"
#include "Engine.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "TransformComponent.h"

#include "Engine.h"

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

    int Engine::Run(HWND mainWindow)
    {
        if (!LoadLevel())
        {
            return -1;
        }

        GAL::RendererD3D12& renderer = GAL::RendererD3D12::GetRenderer();
        if (!renderer.Init(g_hWnd)) {
            ODERROR("Failed to initialize the renderer");
            return -1;
        }



        GAL::InputManager::Instance().Init(g_hWnd);
        GAL::InputManager::Instance().AddListener(&renderer);

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
                GAL::InputManager::Instance().ProcessEvent(windowEvent);
            }

            deltaTimeMilis = (float)timerCounter.readDeltaTimeMillis();
            //GAL::odprintf("deltaTimeMilis=%f", deltaTimeMilis);
            renderer.RenderFrame(deltaTimeMilis);

            ++frameCnt;
        }

        renderer.CleanUp();

        perfCounter.stop();
        //FILE* pFile = fopen("test.txt", "w");
        perfCounter.dump(frameCnt);// , pFile);
        double seconds = perfCounter.getElapsedSeconds();
        GAL::odprintf("frameCnt=%u FPS=%.2f\n", frameCnt, (double)frameCnt / seconds);
        //fclose(pFile);
        return 0;
    }

    bool Engine::LoadLevel(const char *levelName = nullptr)
    {
        //We don't support loading levels by name, yet.
        assert(levelname == nullptr);

        //srand((unsigned int)time(NULL));
        srand(666);


        GAL::MeshCache& meshCache = GAL::ResourceManager::GetMeshCache();
        GAL::Registry& registry = GAL::Engine::Instance().GetRegistry();

        //Create all the triangle entities.
        const int kMaxTriangles = 1000;
        std::stringstream meshResourceNameStream;
        std::string meshResourceName;
        for (int i = 0; i < kMaxTriangles; i++)
        {
            //Create the mesh resource
            meshResourceNameStream << "assets/mesh/triangle" << i;
            meshResourceName = meshResourceNameStream.str();
            auto meshResId = GAL::ResourceManager::GetResourceId(meshResourceName.c_str());

            float rndRed = randFloat(0.2f, 1.0f);
            float rndGreen = randFloat(0.2f, 1.0f);
            float rndBlue = randFloat(0.2f, 1.0f);
            if (!meshCache.load<GAL::TriangleMeshLoader>(meshResId, 0.5f, rndRed, rndGreen, rndBlue))
            {
                ODERROR("Failed to create mesh for triangle %d", i);
                return;
            }

            auto meshResHandle = meshCache.handle(meshResId);

            auto newEntity = registry.create();

            //Add the mesh component.
            GAL::MeshComponent& meshComponent = registry.assign<GAL::MeshComponent>(newEntity);
            meshComponent.m_meshName = meshResourceName;
            meshComponent.m_meshId = meshResId;

            //Add the transform component
            GAL::TransformComponent& transformComponent = registry.assign<GAL::TransformComponent>(newEntity);
            ::GenerateRandomWorldMatrix(transformComponent.m_matrix, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 10.0f);
        }

        //Create the camera entity



        //GAL::RenderNode* node = new GAL::RenderNode();
        //renderer.AddRenderNode(node);
    }

} //namespace GAL