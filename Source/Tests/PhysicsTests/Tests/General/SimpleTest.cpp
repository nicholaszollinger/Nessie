// SimpleTest.cpp
#include "SimpleTest.h"

#include "../../Layers.h"
#include "Nessie/Physics/Collision/Shapes/BoxShape.h"

SimpleTest::~SimpleTest()
{
    m_pPhysicsScene->SetBodyActivationListener(nullptr);
}

void SimpleTest::Init()
{
    m_pPhysicsScene->SetBodyActivationListener(&m_bodyActivationListener);

    CreateFloor();

    nes::ConstStrongPtr<nes::Shape> boxShape = NES_NEW(nes::BoxShape(nes::Vec3(0.5f, 1.f, 2.f)));

    // Dynamic Body 1
    m_pBodyInterface->CreateAndAddBody(nes::BodyCreateInfo(boxShape, nes::RVec3(0, 10, 0), nes::Quat::Identity(), nes::EBodyMotionType::Dynamic, PhysicsLayers::kMoving), nes::EBodyActivationMode::Activate);
    
    // Dynamic Body 2
    m_pBodyInterface->CreateAndAddBody(nes::BodyCreateInfo(boxShape, nes::RVec3(5, 10, 0), nes::Quat::FromAxisAngle(nes::Vec3::AxisX(), 0.25f * nes::math::Pi()), nes::EBodyMotionType::Dynamic, PhysicsLayers::kMoving), nes::EBodyActivationMode::Activate);
        
    // Dynamic Body 3
    m_pBodyInterface->CreateAndAddBody(nes::BodyCreateInfo(boxShape, nes::RVec3(10, 10, 0), nes::Quat::FromAxisAngle(nes::Vec3::AxisZ(), 0.25f * nes::math::Pi()), nes::EBodyMotionType::Dynamic, PhysicsLayers::kMoving), nes::EBodyActivationMode::Activate);
}