// Application.h
#pragma once
#include "Nessie/Application/Application.h"

#include "Layers.h"
#include "Nessie/Jobs/JobSystemThreadPool.h"
#include "Nessie/Core/Memory/StackAllocator.h"
#include "Tests/Test.h"
#include "Utils/ContactListenerImpl.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Application for running different Physics Tests. 
//----------------------------------------------------------------------------------------------------
class TestApplication final : public nes::Application
{
public:
    TestApplication(const nes::ApplicationDesc& appDesc) : nes::Application(appDesc) {}

protected:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Pause/unpause the simulation. 
    //----------------------------------------------------------------------------------------------------
    void                    Pause(const bool shouldPause) { m_isPaused = shouldPause; }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Set simulation state to single-step.
    //----------------------------------------------------------------------------------------------------
    void                    SetSingleStep() { m_isPaused = true; m_singleStep = true;}

    //----------------------------------------------------------------------------------------------------
    /// @brief : Restores the camera to the returned value of GetInitialCamera(). 
    //----------------------------------------------------------------------------------------------------
    void                    ResetCamera();
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the current camera state, in world space.
    //----------------------------------------------------------------------------------------------------
    const CameraState&      GetCamera() const { return m_worldCamera; }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the initial camera state. 
    //----------------------------------------------------------------------------------------------------
    void                    GetInitialCamera(CameraState& outState) const;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get a camera pivot point and orientation in world space.
    //----------------------------------------------------------------------------------------------------
    nes::Mat44              GetCameraPivot(float cameraHeading, float cameraPitch) const; 
    
private:
    /// Application overrides.
    virtual bool            Internal_AppInit() override;
    virtual void            Internal_AppUpdate(const float deltaTime) override;
    virtual void            Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& swapchainImageView) override;
    virtual void            Internal_AppShutdown() override;
    virtual void            PushEvent(nes::Event& e) override;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update the current test.
    //----------------------------------------------------------------------------------------------------
    bool                    Update(const float worldDeltaTime);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Start running a new test.
    //----------------------------------------------------------------------------------------------------
    void                    StartTest();
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Draw the state of the physics system. 
    //----------------------------------------------------------------------------------------------------
    void                    DrawPhysics();

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update the Physics Scene. 
    //----------------------------------------------------------------------------------------------------
    void                    StepPhysics(nes::JobSystem* pJobSystem);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Update the camera transform. 
    //----------------------------------------------------------------------------------------------------
    void                    UpdateCamera(const float deltaTime);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the heading and pitch from the local space (relative to the camera pivot) camera forward.
    //----------------------------------------------------------------------------------------------------
    void                    GetCameraLocalHeadingAndPitch(float& outHeading, float& outPitch) const;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert the local space camera to the world space camera.
    //----------------------------------------------------------------------------------------------------
    void                    ConvertCameraLocalToWorld(const float cameraHeading, const float cameraPitch);

    //----------------------------------------------------------------------------------------------------
    /// @brief :  Get the scale factor for this world. Used to boost camera speed kj.
    //----------------------------------------------------------------------------------------------------
    float                   GetWorldScale() const;

private:
    int                     m_maxConcurrentJobs = std::thread::hardware_concurrency();  // How many jobs to run in parallel.
    float                   m_updateFrequency = 60.f;
    int                     m_collisionSteps = 1;
    nes::StackAllocator*    m_pAllocator = nullptr;
    nes::JobSystem*         m_pJobSystem = nullptr;
    nes::PhysicsScene*      m_pPhysicsScene = nullptr;
    BroadPhaseLayerInterfaceTest m_broadPhaseLayerInterface;
    CollisionLayerPairFilterTest m_layerPairFilter;
    CollisionVsBroadPhaseLayerFilterTest m_collisionVsBroadPhaseLayerFitler;
    ContactListenerImpl*    m_pContactListener = nullptr;
    CameraState             m_worldCamera;
    CameraState             m_localCamera;
    Test*                   m_pTest = nullptr;
    nes::Vec3               m_inputMovement{}; 
    nes::Vec2               m_inputRotation{};
    float                   m_requestedDeltaTime = 0.f;
    float                   m_residualDeltaTime = 0.f;
    bool                    m_cameraRotationEnabled = false;
    bool                    m_isPaused = false;
    bool                    m_singleStep = false;
    
};