// EmptyShape.h
#pragma once
#include "Shape.h"

namespace nes
{
    class EmptyShapeSettings final : public ShapeSettings
    {
    public:
        Vec3 m_centerOfMass = Vec3::Zero();
        
        EmptyShapeSettings() = default;
        explicit EmptyShapeSettings(const Vec3& centerOfMass) : m_centerOfMass(centerOfMass) {}

        virtual ShapeResult Create() const override;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : An Empty Shape has not volume and collides with nothing.
    ///     Possible use-cases:
    ///     - As a placeholder for a shape that will be created later. E.g. if yo first need to create a
    ///       body and only then know what shape it will have.
    ///     - If you need a kinematic body to attach a constraint to, but you don't wa nt the body to collide
    ///       with anything.
    ///
    /// @note : If possible, you should also put your body in a collision layer that doesn't collide with
    ///     anything. This ensures that collisions will be filtered out at a broad phase level instead of
    ///     a narrow phase level - this is more efficient.
    //----------------------------------------------------------------------------------------------------
    class EmptyShape final : public Shape
    {
    public:
        EmptyShape() : Shape(EShapeType::Empty, EShapeSubType::Empty) {}
        explicit EmptyShape(const Vec3& centerOfMass) : Shape(EShapeType::Empty, EShapeSubType::Empty), m_centerOfMass(centerOfMass) {}
        EmptyShape(const EmptyShapeSettings& settings, ShapeResult& outResult);

        virtual Vec3            GetCenterOfMass() const override                    { return m_centerOfMass; }
        virtual AABox           GetLocalBounds() const override                     { return { Vec3::Zero(), Vec3::Zero() }; }
        virtual unsigned        GetSubSShapeIDBitsRecursive() const override        { return 0; }
        virtual float           GetInnerRadius() const override                     { return 0.f; }
        virtual MassProperties  GetMassProperties() const override;
        virtual Vec3            GetSurfaceNormal(const SubShapeID&, const Vec3&) const override { return Vec3::Zero(); }
        // [TODO]: PhysicsMaterial
        // [TODO]: SubmergedVolume

        virtual bool            CastRay(const RayCast&, const SubShapeIDCreator&, RayCastResult&) const override { return false; }
        virtual void            CastRay(const RayCast&, const RayCastSettings&, const SubShapeIDCreator&, CastRayCollector&, const ShapeFilter&) const override { }
        virtual void            CollidePoint(const Vec3&, const SubShapeIDCreator&, CollidePointCollector&, const ShapeFilter&) const override { }
        virtual void            GetTrianglesStart(GetTrianglesContext&, const AABox&, const Vec3&, const Quat&, const Vec3&) const override { }
        virtual int             GetTrianglesNext(GetTrianglesContext&, int, Float3*) const override { return 0; }
        virtual float           GetVolume() const override { return 0.f; }
        virtual bool            IsValidScale(const Vec3&) const override { return true; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Register Shape functions with the registry. 
        //----------------------------------------------------------------------------------------------------
        static void             Register();

    private:
        Vec3                    m_centerOfMass = Vec3::Zero();
    };
}