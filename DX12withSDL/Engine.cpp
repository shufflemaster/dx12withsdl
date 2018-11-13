#include "pch.h"

#include "PerfCounter.h"
#include "TimeCounter.h"
#include "CommonHelpers.h"
//#include "VertexTypes.h"
#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "HashedString.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "Universe.h"

#include "Systems/InputSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/RenderSystem.h"
#include "ResourceTypes/Mesh.h"
#include "ResourceLoaders/TriangleMeshLoader.h"
#include "ResourceLoaders/SphereMeshLoader.h"
#include "ResourceLoaders/PlaneMeshLoader.h"

using namespace DirectX;

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

#if 0
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
#endif
        //*************************************************
        //Create sphere
        if (1)
        {
            auto sphereEntity = m_universe.CreteEntity();
            MeshComponent& meshComponent = registry.assign<MeshComponent>(sphereEntity);

            const float radius = 1.0f;
            meshComponent.m_meshHandle = meshCache.AddResource<SphereMeshLoader>("assets/mesh/sphere", radius, 5.0f, 1, -1, 1);
            if (meshComponent.m_meshHandle < 0)
            {
                ODERROR("Failed to create mesh for spehre");
                return false;
            }

            //Add the transform component
            TransformComponent& tc = registry.assign<TransformComponent>(sphereEntity);
            XMMATRIX identityMat = XMMatrixTranslation(0.0f, radius, 0.0f);
            XMStoreFloat4x4A(&tc.m_matrix, identityMat);
        }
        //*************************************************

        //*************************************************
        //Create plane.
        if (1) 
        {
            auto planeEntity = m_universe.CreteEntity();
            MeshComponent& meshComponent = registry.assign<MeshComponent>(planeEntity);

            meshComponent.m_meshHandle = meshCache.AddResource<PlaneMeshLoader>("assets/mesh/plane", 20.0f, 20, 20.0f, 20, XMFLOAT3{0, 1, 0});
            if (meshComponent.m_meshHandle < 0)
            {
                ODERROR("Failed to create mesh for spehre");
                return false;
            }

            //Add the transform component
            TransformComponent& tc = registry.assign<TransformComponent>(planeEntity);
            XMMATRIX identityMat = XMMatrixIdentity();
            XMStoreFloat4x4A(&tc.m_matrix, identityMat);
        }
        //*************************************************

        //Create the camera entity
        auto camEntity = m_universe.CreteEntity();
        //camera Transform.
        TransformComponent& transformComponent = registry.assign<TransformComponent>(camEntity);
        transformComponent.SetRight(   XMFLOAT4A(1.0f, 0.0f,  0.0f, 0.0f));
        transformComponent.SetUp(      XMFLOAT4A(0.0f, 1.0f,  0.0f, 0.0f));
        transformComponent.SetForward( XMFLOAT4A(0.0f, 0.0f,  1.0f, 0.0f));
        transformComponent.SetPosition(XMFLOAT4A(0.0f, 4.0f, -20.0f, 1.0f));
        CameraComponent& cameraComponent = registry.assign<CameraComponent>(camEntity);
        cameraComponent.m_fieldOfViewDegrees = 45.0f;
        cameraComponent.m_nearClipDistance = 0.1f;
        cameraComponent.m_farClipDistance = 1000.0f;
        cameraComponent.m_translationSpeed = 4.0f; //4 world units per second.
        cameraComponent.m_rotationSpeed = 30.0f; //90 degrees per second.
        cameraComponent.m_isActive = true;

        //Create the entity used to register input events we care about.
        //Could be attached directly to the camera, but ideally we want something that simply generates
        //Input events for whomever cares.
        auto inputEntity = m_universe.CreteEntity();
        InputComponent& inputComponent = registry.assign<InputComponent>(inputEntity);
        auto evtId = inputComponent.AddEventNameHelper("MoveForward");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_W, 1.0f, 0.0f);
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_S, -1.0f, 0.0f);
        evtId = inputComponent.AddEventNameHelper("MoveRight");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_D, 1.0f, 0.0f);
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_A, -1.0f, 0.0f);
        evtId = inputComponent.AddEventNameHelper("MoveUp");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_Q, 1.0f, 0.0f);
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_E, -1.0f, 0.0f);
        evtId = inputComponent.AddEventNameHelper("MouseMiddleButton");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::MOUSE_BUTTON_MIDDLE, 1.0f, 0.0f);
        evtId = inputComponent.AddEventNameHelper("MouseRightButton");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::MOUSE_BUTTON_RIGHT, 1.0f, 0.0f);
        //MouseDeltaX + MouseMiddleButton = MoveRight
        //MouseDeltaX + MouseRightButton = MoveYaw
        evtId = inputComponent.AddEventNameHelper("MouseDeltaX");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::MOUSE_DELTA_X, 1.0f, 0.001f);
        //MouseDeltaY + MouseMiddleButton = MoveUp
        //MouseDeltaY + MouseRightButton = MovePitch
        evtId = inputComponent.AddEventNameHelper("MouseDeltaY");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::MOUSE_DELTA_Y, -1.0f, 0.001f);
        evtId = inputComponent.AddEventNameHelper("RotatePitch");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_UP, 0.02f, 0.0f);
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_DOWN, -0.02f, 0.0f);
        evtId = inputComponent.AddEventNameHelper("RotateYaw");
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_LEFT, -0.02f, 0.0f);
        inputComponent.AddEventInputItemHelper(evtId, InputComponent::eInputName::KEYBOARD_RIGHT, 0.02f, 0.0f);

        return true;
    }

    bool Engine::CreateAndInitSystems(HWND hWnd)
    {
        //The order in which we add it here determines the order of Tick Update.
        m_inputSystem = static_cast<InputSystem*>(m_universe.CreateAndAddSystem<InputSystem>(hWnd));
        m_cameraSystem = static_cast<CameraSystem*>(m_universe.CreateAndAddSystem<CameraSystem>(&m_renderer));
        m_renderSystem = static_cast<RenderSystem*>(m_universe.CreateAndAddSystem<RenderSystem>(&m_renderer));

        m_universe.InitializeSystems();

        return true;
    }

} //namespace GAL