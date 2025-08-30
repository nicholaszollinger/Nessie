// Test.h
#pragma once
#include "Nessie/Graphics/Camera.h"
#include "Nessie/Physics/PhysicsScene.h"
#include "Nessie/Input/InputManager.h"

struct CameraState
{
    CameraState() : m_position(nes::RVec3::Zero()), m_forward(0.f, 0.f, 1.f), m_up(0.f, 1.f, 0.f), m_fovY(nes::math::DegreesToRadians() * 70.f) {}
    
    nes::RVec3  m_position;
    nes::Vec3   m_forward;
    nes::Vec3   m_up;
    float       m_fovY;    
};

class Test
{
public:
    virtual ~Test() = default;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Initialize the test. 
    //----------------------------------------------------------------------------------------------------
    virtual void            Init() {}

    //----------------------------------------------------------------------------------------------------
    /// @brief : Number use to scale the terrain and camera movement.
    //----------------------------------------------------------------------------------------------------
    virtual float           GetWorldScale() const { return 1.f; }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Set the Physics Scene.
    //----------------------------------------------------------------------------------------------------
    virtual void            SetPhysicsScene(nes::PhysicsScene* pScene) { m_pPhysicsScene = pScene; m_pBodyInterface = &pScene->GetBodyInterface(); }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Set the job system.
    //----------------------------------------------------------------------------------------------------
    void                    SetJobSystem(nes::JobSystem* pJobSystem) { m_pJobSystem = pJobSystem; }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Set the allocator to use for the test.
    //----------------------------------------------------------------------------------------------------
    void                    SetAllocator(nes::StackAllocator* pAllocator) { m_pAllocator = pAllocator; }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : If this test implements a contact listener, it should be returned here.
    //----------------------------------------------------------------------------------------------------
    virtual nes::ContactListener* GetContactListener() { return nullptr; }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Process input.
    //----------------------------------------------------------------------------------------------------
    virtual void            ProcessInput([[maybe_unused]] const float deltaTime, [[maybe_unused]] const CameraState& camera) {}

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update the test, before the physics update.
    //----------------------------------------------------------------------------------------------------
    virtual void            PrePhysicsUpdate([[maybe_unused]] const float deltaTime, [[maybe_unused]] const CameraState& camera) {}

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update the test, after the physics update. 
    //----------------------------------------------------------------------------------------------------
    virtual void            PostPhysicsUpdate([[maybe_unused]] const float deltaTime) {}

    //----------------------------------------------------------------------------------------------------
    /// @brief : Override to specify the initial camera settings.
    //----------------------------------------------------------------------------------------------------
    virtual void            GetInitialCamera([[maybe_unused]] CameraState& outState) const {}

    //----------------------------------------------------------------------------------------------------
    /// @brief : Override to specify a camera pivot point and orientation in world space.
    //----------------------------------------------------------------------------------------------------
    virtual nes::Mat44      GetCameraPivot([[maybe_unused]] const float heading, [[maybe_unused]] const float pitch) const { return nes::Mat44::Identity(); }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Force the application to restart the test. 
    //----------------------------------------------------------------------------------------------------
    void                    RestartTest()                                   { m_needsRestart = true; }
    bool                    NeedsRestart() const                            { return m_needsRestart; }

protected:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create a static floor body.
    //----------------------------------------------------------------------------------------------------
    nes::Body&              CreateFloor(const float size = 200.f);
    
    //----------------------------------------------------------------------------------------------------
    // [TODO]: 
    /// @brief : Add a label to a Body. 
    //----------------------------------------------------------------------------------------------------
    //void SetBodyLabel(const nes::BodyID& bodyID, const std::string& label) { m_bodyLabels[bodyID] = label; }

protected:
    nes::JobSystem*         m_pJobSystem = nullptr;
    nes::PhysicsScene*      m_pPhysicsScene = nullptr;
    nes::BodyInterface*     m_pBodyInterface = nullptr;
    nes::StackAllocator*    m_pAllocator = nullptr;

private:
    // [TODO]: 
    //using BodyLabels = std::unordered_map<nes::BodyID, std::string>;
    
    bool                    m_needsRestart = false;
    // [TODO]: 
    //BodyLabels              m_bodyLabels;
};
