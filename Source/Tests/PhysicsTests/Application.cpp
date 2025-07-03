#include "Application.h"

#include "Nessie/Core/Jobs/JobSystemThreadPool.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Physics/Collision/CollisionSolver.h"
#include "Nessie/Physics/Collision/Shapes/BoxShape.h"
#include "Nessie/Physics/Collision/Shapes/EmptyShape.h"
#include "Tests/General/SimpleTest.h"

static constexpr uint kNumBodies = 10240;
static constexpr uint kNumBodyMutexes = 0; // Autodetect
static constexpr uint kMaxBodyPairs = 65536;
static constexpr uint kMaxContactConstraints = 20480;

void TestApplication::ResetCamera()
{
    m_localCamera = CameraState();
    GetInitialCamera(m_localCamera);

    // Convert to world space
    float heading, pitch;
    GetCameraLocalHeadingAndPitch(heading, pitch);
    ConvertCameraLocalToWorld(heading, pitch);
}

void TestApplication::GetInitialCamera(CameraState& outState) const
{
    // Default if the test doesn't override it.
    outState.m_position = GetWorldScale() * nes::Vec3(30.f, 10.f, 30.f);
    outState.m_forward = -nes::Vec3(outState.m_position).Normalized();

    m_pTest->GetInitialCamera(outState);
}

nes::Mat44 TestApplication::GetCameraPivot(const float cameraHeading, const float cameraPitch) const
{
    return m_pTest->GetCameraPivot(cameraHeading, cameraPitch);
}

bool TestApplication::Internal_Init()
{
    nes::CollisionSolver::Internal_Init();
    nes::ConvexShape::Register();
    nes::BoxShape::Register();
    nes::EmptyShape::Register();

    //nes::LoggerRegistry::Instance().GetDefaultLogger()->SetLevel(nes::ELogLevel::Trace);
    
    // Allocate temp memory
    m_pAllocator = NES_NEW(nes::StackAllocator(static_cast<size_t>(32 * 1024 * 1024)));

    // Create the job system
    m_pJobSystem = NES_NEW(nes::JobSystemThreadPool(nes::physics::kMaxPhysicsJobs, nes::physics::kMaxPhysicsBarriers, m_maxConcurrentJobs - 1));

    // [TODO]: Start Test needs to take in a class parameter.
    StartTest();
    
    ResetCamera();
    
    return true;
}

void TestApplication::RunFrame(const double deltaTime)
{
    float worldDeltaTime = 0.f;
    if (m_requestedDeltaTime <= 0.f)
    {
        // If no fixed frequency update is requested, update with a variable time step.
        worldDeltaTime = !m_isPaused || m_singleStep? static_cast<float>(deltaTime) : 0.f;
        m_residualDeltaTime = 0.f;
    }
    else
    {
        // Else we are using fixed time steps.
        if (m_singleStep)
        {
            worldDeltaTime = m_requestedDeltaTime;
        }
        else if (!m_isPaused)
        {
            // Calculate how much time has passed since the last render.
            worldDeltaTime = static_cast<float>(deltaTime) + m_residualDeltaTime;
            if (worldDeltaTime < m_requestedDeltaTime)
            {
                // Too soon, set the residual time and don't update.
                m_residualDeltaTime = worldDeltaTime;
                worldDeltaTime = 0.f;
            }
            else
            {
                // Update and clamp the residual time to a full update to avoid spiral of death
                m_residualDeltaTime = nes::math::Min(m_requestedDeltaTime, worldDeltaTime - m_requestedDeltaTime);
                worldDeltaTime = m_requestedDeltaTime;
            }
        }
    }
    m_singleStep = false;

    // Update with the calculated world deltaTime
    if (!Update(worldDeltaTime))
    {
        Quit();
        return;
    }

    UpdateCamera(static_cast<float>(deltaTime));
}

bool TestApplication::Update(const float deltaTime)
{
    // Reinitialize the job system if the concurrency setting changed.
    //    if (m_maxConcurrentJobs != m_pJobSystem->GetMaxConcurrency())
    //      checked_cast<nes::JobSystemThreadPool*>(m_pJobSystem)->SetNumThreads(m_maxConcurrentJobs - 1);

    if (m_pTest->NeedsRestart())
    {
        StartTest();
        return true;
    }
    
    if (deltaTime > 0.f)
    {
        m_pTest->ProcessInput(deltaTime, GetCamera());
    
        DrawPhysics();
        StepPhysics(m_pJobSystem);
    }
    
    return true;
}

void TestApplication::OnAppShutdown()
{
    NES_SAFE_DELETE(m_pTest);
    NES_SAFE_DELETE(m_pContactListener);
    NES_SAFE_DELETE(m_pPhysicsScene);
    
    NES_SAFE_DELETE(m_pAllocator);
    NES_SAFE_DELETE(m_pJobSystem);
}

void TestApplication::PushEvent(nes::Event& e)
{
    // Mouse Event
    if (e.GetEventID() == nes::MouseButtonEvent::GetStaticEventID())
    {
        auto& mouseButtonEvent = checked_cast<nes::MouseButtonEvent&>(e);
        // Right click to enable camera turning.
        if (mouseButtonEvent.GetButton() == nes::EMouseButton::Right)
        {
            if (mouseButtonEvent.GetAction() == nes::EMouseAction::Pressed)
            {
                m_cameraRotationEnabled = true;
                nes::InputManager::SetCursorMode(nes::ECursorMode::Disabled);
            }
                    
            else if (mouseButtonEvent.GetAction() == nes::EMouseAction::Released)
            {
                m_cameraRotationEnabled = false;
                nes::InputManager::SetCursorMode(nes::ECursorMode::Visible);
            }
        }
    }

    // Key Event
    else if (e.GetEventID() == nes::KeyEvent::GetStaticEventID())
    {
        auto& keyEvent = checked_cast<nes::KeyEvent&>(e);

        if (keyEvent.GetAction() == nes::EKeyAction::Pressed)
        {
            // P to Pause
            if (keyEvent.GetKeyCode() == nes::EKeyCode::P)
            {
                m_isPaused = !m_isPaused;
            }

            // Escape to Quit
            else if (keyEvent.GetKeyCode() == nes::EKeyCode::Escape)
            {
                Quit();
            }
        }
    }
}

void TestApplication::StartTest()
{
    // Store old gravity
    const nes::Vec3 oldGravity = m_pPhysicsScene != nullptr? m_pPhysicsScene->GetGravity() : nes::Vec3(0.f, -9.81f, 0.f);

    // Discard old test.
    NES_DELETE(m_pTest);
    NES_DELETE(m_pPhysicsScene);
    NES_DELETE(m_pContactListener);

    // Create the new physics scene.
    m_pPhysicsScene = NES_NEW(nes::PhysicsScene());
    nes::PhysicsScene::CreateInfo info;
    info.m_maxBodies = kNumBodies;
    info.m_numBodyMutexes = kNumBodyMutexes;
    info.m_maxNumBodyPairs = kMaxBodyPairs;
    info.m_maxNumContactConstraints = kMaxContactConstraints;
    info.m_pCollisionLayerPairFilter = &m_layerPairFilter;
    info.m_pCollisionVsBroadPhaseLayerFilter = &m_collisionVsBroadPhaseLayerFitler;
    info.m_pLayerInterface = &m_broadPhaseLayerInterface;
    m_pPhysicsScene->Init(info);

    // Restore gravity
    m_pPhysicsScene->SetGravity(oldGravity);

    // Set a new test:
    m_pTest = NES_NEW(SimpleTest());
    m_pTest->SetPhysicsScene(m_pPhysicsScene);
    m_pTest->SetJobSystem(m_pJobSystem);
    m_pTest->SetAllocator(m_pAllocator);

    // Update Contact listener.
    m_pContactListener = NES_NEW(ContactListenerImpl());
    m_pContactListener->SetNextListener(m_pTest->GetContactListener());
    m_pPhysicsScene->SetContactListener(m_pContactListener);

    // Initialize the Test.
    m_pTest->Init();

    // Optimize the broadphase to make the first update fast.
    m_pPhysicsScene->OptimizeBroadPhase();

    ResetCamera();

    // Start paused.
    Pause(true);
}

void TestApplication::DrawPhysics()
{
    // [TODO]: 
}

void TestApplication::StepPhysics(nes::JobSystem* pJobSystem)
{
    const float deltaTime = 1.f / m_updateFrequency;

    // Pre Update
    m_pTest->PrePhysicsUpdate(deltaTime, GetCamera());

    // Step the world (with a fixed frequency).
    m_pPhysicsScene->Update(deltaTime, m_collisionSteps, m_pAllocator, pJobSystem);

    //NES_LOG("Finished Step");
    
    // Post Update
    m_pTest->PostPhysicsUpdate(deltaTime);
}

void TestApplication::UpdateCamera(const float deltaTime)
{
    m_inputMovement = nes::Vec3::Zero();
    m_inputRotation = nes::Vec2::Zero();

    // Process Movement:
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::W))
        m_inputMovement.z += 1.f;
            
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::S))
        m_inputMovement.z -= 1.f;
            
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::A))
        m_inputMovement.x -= 1.f;
        
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::D))
        m_inputMovement.x += 1.f;
        
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::Space))
        m_inputMovement.y += 1.f;  
            
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::LeftControl) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightControl))
        m_inputMovement.y -= 1.f;
    
    // Normalize movement vector
    m_inputMovement.NormalizedOr(nes::Vec3::Zero());
        
    // Process Rotation:
    if (m_cameraRotationEnabled)
    {
        const nes::Vec2 delta = nes::InputManager::GetCursorDelta();
        m_inputRotation.x = delta.y;
        m_inputRotation.y = delta.x;
        m_inputRotation.Normalize();
    }

    // If there is enough input to warrant an update:
    if (m_inputMovement.LengthSqr() > 0.f || m_inputRotation.LengthSqr() > 0.f)
    {
        float speed = 20.f * GetWorldScale() * deltaTime;
        if (nes::InputManager::IsKeyDown(nes::EKeyCode::LeftShift) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightShift))
            speed *= 10.f;
        
        // Update position
        const nes::Vec3 right = m_localCamera.m_forward.Cross(m_localCamera.m_up);
        m_localCamera.m_position += right * m_inputMovement.x * speed;
        m_localCamera.m_position += m_localCamera.m_forward * m_inputMovement.z * speed;
        m_localCamera.m_position += nes::Vec3::Up() * m_inputMovement.y * speed;

        // Update forward
        float heading, pitch;
        GetCameraLocalHeadingAndPitch(heading, pitch);
        heading += nes::math::DegreesToRadians() * m_inputRotation.x * 0.5f;
        pitch = nes::math::Clamp(pitch - nes::math::DegreesToRadians() * (m_inputRotation.y * 0.5f), -0.49f * nes::math::Pi(), 0.49f * nes::math::Pi());
        m_localCamera.m_forward = nes::Vec3(nes::math::Cos(pitch) * nes::math::Cos(heading), nes::math::Sin(pitch), nes::math::Cos(pitch) * nes::math::Sin(heading));

        // Convert to local space
        ConvertCameraLocalToWorld(heading, pitch);
    }
}

void TestApplication::GetCameraLocalHeadingAndPitch(float& outHeading, float& outPitch) const
{
    outHeading = nes::math::ATan2(m_localCamera.m_forward.z, m_localCamera.m_forward.x);
    outPitch = nes::math::ATan2(m_localCamera.m_forward.y, nes::Vec3(m_localCamera.m_forward.x, 0.f, m_localCamera.m_forward.z).Length());
}

void TestApplication::ConvertCameraLocalToWorld(const float cameraHeading, const float cameraPitch)
{
    // Convert local to world space using the camera pivot.
    nes::Mat44 pivot = GetCameraPivot(cameraHeading, cameraPitch);
    m_worldCamera = m_localCamera;
    m_worldCamera.m_position = pivot * m_localCamera.m_position;
    m_worldCamera.m_forward = pivot.Multiply3x3(m_localCamera.m_forward);
    m_worldCamera.m_up = pivot.Multiply3x3(m_localCamera.m_up);
}

float TestApplication::GetWorldScale() const
{
    return m_pTest != nullptr? m_pTest->GetWorldScale() : 1.f;
}

nes::Application* nes::CreateApplication([[maybe_unused]] const nes::CommandLineArgs& args)
{
    nes::ApplicationProperties appProps;
    appProps.m_appName = "Physics Tests";
    appProps.m_commandLineArgs = args;
    appProps.m_isMultithreaded = true;

    nes::WindowProperties windowProps;
    windowProps.m_extent = { .m_width = 1600, .m_height = 900};
    windowProps.m_isMinimized = false;
    windowProps.m_isResizable = false;
    windowProps.m_label = "Physics Tests";
    windowProps.m_vsyncEnabled = false;
    windowProps.m_windowMode = EWindowMode::Windowed;
    
    return NES_NEW(TestApplication(std::move(appProps), std::move(windowProps)));
}

NES_MAIN()