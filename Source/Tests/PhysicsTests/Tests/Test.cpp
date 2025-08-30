// Test.cpp
#include "Test.h"

#include "../Layers.h"
#include "Nessie/Physics/Collision/Shapes/BoxShape.h"

nes::Body& Test::CreateFloor(const float size)
{
    using namespace nes;
    const float scale = GetWorldScale();

    Body& floor = *m_pBodyInterface->CreateBody(BodyCreateInfo(new BoxShape(scale * Vec3(0.5f * size, 1.f, 0.5f * size), 0.f), RVec3(scale * Vec3(0.f, -1.f, 0.f)), Quat::Identity(), EBodyMotionType::Static, PhysicsLayers::kNonMoving));
    m_pBodyInterface->AddBody(floor.GetID(), EBodyActivationMode::DontActivate);
    return floor;
}