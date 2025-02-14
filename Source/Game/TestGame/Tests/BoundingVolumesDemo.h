// BoundingVolumesDemo.h
#pragma once
#include "../Demo.h"
#include "Math/ConvexHull2.h"
#include "Math/Polygon2.h"
#include "Math/Transform.h"

enum class BoundingVolumeType
{
    AABB,
    //OBB,
    Sphere,
    //Capsule,
    ConvexHull,
    Num,
};

class BoundingVolumesDemo final : public Demo
{
    inline static std::string s_boundingTypenames[]
    {
        "AABB",
        //"OBB",
        "Sphere",
        //"Capsule",
        "ConvexHull",
    };
    
    nes::Polygon2D m_polyA{};
    nes::Polygon2D m_polyB{};
    nes::ConvexHull2D m_convexHullA{};
    nes::ConvexHull2D m_convexHullB{};
    nes::Transform2D m_transformA{};
    nes::Transform2D m_transformB{};
    BoundingVolumeType m_boundingType = BoundingVolumeType::AABB;
    
    // [TODO]: Allow for two different types.
    // BoundingVolumeType m_boundingTypeB;
    
public:
    virtual bool Init() override;
    virtual void Reset() override;
    virtual void Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix) override;
    virtual void RenderImGui() override;
    [[nodiscard]] virtual const char* GetName() const override { return "Bounding Volumes"; }
    
};
