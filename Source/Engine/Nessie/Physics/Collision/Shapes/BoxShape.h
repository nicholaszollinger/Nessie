// BoxShape.h
#pragma once
#include "ConvexShape.h"
#include "Physics/PhysicsSettings.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings that create a Box Shape.
    //----------------------------------------------------------------------------------------------------
    class BoxShapeSettings final : public ConvexShapeSettings
    {
    public:
        Vector3 m_halfExtent    = Vector3::Zero(); /// Half the size of the box, including the convex radius.
        float   m_convexRadius  = 0.f;
        
        BoxShapeSettings() = default;
        explicit BoxShapeSettings(const Vector3& halfExtent, float convexRadius = physics::kDefaultConvexRadius);

        //----------------------------------------------------------------------------------------------------
        /// @see : ShapeSettings::Create()
        //----------------------------------------------------------------------------------------------------
        virtual ShapeResult Create() const override;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Box, centered around the origin. 
    //----------------------------------------------------------------------------------------------------
    class BoxShape final : public nes::ConvexShape
    {
        /// Class for GetSupportFunction()
        class Box;

        Vector3 m_halfExtent    = Vector3::Zero(); /// Half the size of the box, including the convex radius.
        float   m_convexRadius  = 0.f;
        
    public:
        BoxShape() : ConvexShape(ShapeSubType::Box) {}
        BoxShape(const BoxShapeSettings& settings, ShapeResult& outResult);
        BoxShape(const Vector3& halfExtent, float convexRadius = physics::kDefaultConvexRadius);
        
        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetLocalBounds()
        //----------------------------------------------------------------------------------------------------
        virtual AABox           GetLocalBounds() const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetInnerRadius()
        //----------------------------------------------------------------------------------------------------
        virtual float           GetInnerRadius() const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetMassProperties()
        //----------------------------------------------------------------------------------------------------
        virtual MassProperties  GetMassProperties() const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetSurfaceNormal()
        //----------------------------------------------------------------------------------------------------
        virtual Vector3         GetSurfaceNormal(const SubShapeID& subShapeID, const Vector3& localSurfacePosition) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::CastRay()
        //----------------------------------------------------------------------------------------------------
        virtual bool            CastRay(const RayCast& ray, const SubShapeIDCreator& subShapeIDCreator, RayCastResult& hitResult) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::CastRay()
        //----------------------------------------------------------------------------------------------------
        virtual void            CastRay(const RayCast& ray, const RayCastSettings& settings, const SubShapeIDCreator& subShapeIDCreator, CastRayCollector& collector, const ShapeFilter& shapeFilter) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::CollidePoint()
        //----------------------------------------------------------------------------------------------------
        virtual void            CollidePoint(const Vector3& point, const SubShapeIDCreator& subShapeIDCreator, CollidePointCollector& collector, const ShapeFilter& shapeFilter) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetTrianglesStart()
        //----------------------------------------------------------------------------------------------------
        virtual void            GetTrianglesStart(GetTrianglesContext& context, const AABox& box, const Vector3& positionCOM, const Quat& rotation, const Vector3& scale) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetTrianglesNext()
        //----------------------------------------------------------------------------------------------------
        virtual int             GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested, Float3* outTriangleVertices) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetVolume()
        //----------------------------------------------------------------------------------------------------
        virtual float           GetVolume() const override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the convex radius of this box. 
        //----------------------------------------------------------------------------------------------------
        float                   GetConvexRadius() const { return m_convexRadius; }

        //----------------------------------------------------------------------------------------------------
        /// @see : ConvexShape::GetSupportFunction()
        //----------------------------------------------------------------------------------------------------
        virtual const Support*  GetSupportFunction(SupportMode mode, SupportBuffer& buffer, const Vector3& scale) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : ConvexShape::GetSupportingFace()
        //----------------------------------------------------------------------------------------------------
        virtual void            GetSupportingFace(const SubShapeID& subShapeID, const Vector3& direction, const Vector3& scale, const Mat4& centerOfMassTransform, SupportingFace& outVertices) const override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Register shape functions within the registry. 
        //----------------------------------------------------------------------------------------------------
        static void             Register();
    };
}
