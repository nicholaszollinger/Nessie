#include "Application.h"

#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Physics/Collision/CollisionSolver.h"
#include "Nessie/Physics/Collision/Shapes/BoxShape.h"
#include "Nessie/Physics/Collision/Shapes/EmptyShape.h"
#include "Tests/General/SimpleTest.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/DeviceImage.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"

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

bool TestApplication::Internal_AppInit()
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

void TestApplication::Internal_AppUpdate(const float deltaTime)
{
    float worldDeltaTime = 0.f;
    if (m_requestedDeltaTime <= 0.f)
    {
        // If no fixed frequency update is requested, update with a variable time step.
        worldDeltaTime = !m_isPaused || m_singleStep? deltaTime : 0.f;
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
            worldDeltaTime = deltaTime + m_residualDeltaTime;
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

void TestApplication::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Transition the swapchain image to color attachment optimal:
    {
        nes::ImageMemoryBarrierDesc transitionBarrierDesc = nes::ImageMemoryBarrierDesc()
            .SetLayoutTransition(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal)
            .SetStages(vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .SetAccessFlags({}, vk::AccessFlagBits2::eColorAttachmentWrite);
    
        commandBuffer.TransitionImageLayout(context.GetSwapchainImage().GetVkImage(), transitionBarrierDesc);
    }
    
    // Set up the attachment for rendering:
    nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
        .SetColorTargets(&context.GetSwapchainImageDescriptor());

    // Begin a render
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        // Clear the screen
        nes::ClearDesc clearDesc = nes::ClearDesc()
            .SetColorValue({ 0.01f, 0.01f, 0.01f, 1.0f });
        commandBuffer.ClearRenderTargets(clearDesc);
        
        // [TODO]: 

        // End
        commandBuffer.EndRendering();
    }

    // Transition the image to present layout:
    {
        nes::ImageMemoryBarrierDesc transitionBarrierDesc = nes::ImageMemoryBarrierDesc()
            .SetLayoutTransition(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR)
            .SetStages(vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe)
            .SetAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite, {});
    
        commandBuffer.TransitionImageLayout(context.GetSwapchainImage().GetVkImage(), transitionBarrierDesc);
    }
}

void TestApplication::Internal_AppShutdown()
{
    NES_SAFE_DELETE(m_pTest);
    NES_SAFE_DELETE(m_pContactListener);
    NES_SAFE_DELETE(m_pPhysicsScene);
    
    NES_SAFE_DELETE(m_pAllocator);
    NES_SAFE_DELETE(m_pJobSystem);
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

void TestApplication::PushEvent(nes::Event& e)
{
    // Mouse Event
    if (auto* pMouseEvent = e.Cast<nes::MouseButtonEvent>())
    {
        if (pMouseEvent->GetButton() == nes::EMouseButton::Right)
        {
            // Right click to enable camera turning.
            if (pMouseEvent->GetAction() == nes::EMouseAction::Pressed)
            {
                m_cameraRotationEnabled = true;
                nes::InputManager::SetCursorMode(nes::ECursorMode::Disabled);
            }
                    
            else if (pMouseEvent->GetAction() == nes::EMouseAction::Released)
            {
                m_cameraRotationEnabled = false;
                nes::InputManager::SetCursorMode(nes::ECursorMode::Visible);
            }
        }
    }
    
    // Key Event
    else if (auto* keyEvent = e.Cast<nes::KeyEvent>())
    {
        if (keyEvent->GetAction() == nes::EKeyAction::Pressed)
        {
            // P to Pause
            if (keyEvent->GetKeyCode() == nes::EKeyCode::P)
            {
                m_isPaused = !m_isPaused;
            }

            // Escape to Quit
            else if (keyEvent->GetKeyCode() == nes::EKeyCode::Escape)
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