// ConvexShape.h
#pragma once
#include "Shape.h"
#include "SubShapeID.h"
#include "Core/StaticArray.h"
#include "Math/Vec3.h"

namespace nes
{
    struct CollideShapeSettings;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Abstract class that constructs a Convex Shape. 
    //----------------------------------------------------------------------------------------------------
    class ConvexShapeSettings : public ShapeSettings
    {
    public:
        ConvexShapeSettings() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the density of the object in kg/m^3. 
        //----------------------------------------------------------------------------------------------------
        void SetDensity(const float density) { m_density = density; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the uniform density of the interior of the convex object (kg / m^3) 
        //----------------------------------------------------------------------------------------------------
        float GetDensity() const { return m_density; }

    protected:
        //ConstStrongPtr<PhysicsMaterial> m_pMaterial;  /// Material assigned to this shape
        float m_density = 1000.f;                       /// Uniform density of the interior of the convex object (kg / m^3).
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all convex shapes. Defines a virtual interface.
    //----------------------------------------------------------------------------------------------------
    class ConvexShape : public Shape
    {
        /// Class for GetTrianglesStart/Next().
        class CSGetTrianglesContext;
        
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Function that provides an interface for GJK.
        //----------------------------------------------------------------------------------------------------
        class Support
        {
        public:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Warning! Virtual Destructor will not be called on this object! 
            //----------------------------------------------------------------------------------------------------
            virtual         ~Support() = default;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Calculate the support vector for this convex shape (includes/excludes the convex radius
            ///     depending on how this was obtained).
            ///     The Support Vector is relative to the center of mass of the shape.
            //----------------------------------------------------------------------------------------------------
            virtual Vec3    GetSupport(const Vec3& direction) const = 0;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the Convex radius of the shape. Collision detection on penetrating shapes is much more
            ///     expensive, so you can add a radius around objects to increase the shape. This makes it far less
            ///     likely that they will actually penetrate.
            //----------------------------------------------------------------------------------------------------
            virtual float   GetConvexRadius() const = 0; 
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Buffer to hold a Support object. Used to avoid dynamic memory allocations. 
        //----------------------------------------------------------------------------------------------------
        struct alignas(16) SupportBuffer
        {
            uint8_t m_data[4160];
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : How the GetSupport function should behave. 
        //----------------------------------------------------------------------------------------------------
        enum class ESupportMode
        {
            ExcludeConvexRadius,    /// Return the shape excluding the convex radius, Support::GetConvexRadius will return the convex radius if there is one, but adding this radius may not result in the most accurate/efficient representation of shapes with sharp edges.   
            IncludeConvexRadius,    /// Return the shape including the convex radius, Support::GetSupport includes the convex radius if there is one, Support::GetConvexRadius will return 0
            Default,                /// Use both Support::GetSupport add Support::GetConvexRadius to get a support point that matches the original shape as accurately/efficiently as possible
        };

    public:
        explicit ConvexShape(const EShapeSubType subType) : Shape(EShapeType::Convex, subType) {}
        ConvexShape(const EShapeSubType subType, const ConvexShapeSettings& settings, ShapeResult& outResult);
        //ConvexShape(const ShapeSubType subType, const PhysicsMaterial* pMaterial);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an object that provides the GetSupport function for this shape.
        ///	@param mode : Determines if this support function includes or excludes the convex radius of the
        ///     values returns by this GetSupport function. This improves numerical accuracy of the results.
        ///	@param buffer : Buffer to contain the Support object.
        ///	@param scale : Scales the shape in local space.
        //----------------------------------------------------------------------------------------------------
        virtual const Support*  GetSupportFunction(ESupportMode mode, SupportBuffer& buffer, const Vec3& scale) const = 0;

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
        virtual void            CollidePoint(const Vec3& point, const SubShapeIDCreator& subShapeIDCreator, CollidePointCollector& collector, const ShapeFilter& shapeFilter) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetTrianglesStart()
        //----------------------------------------------------------------------------------------------------
        virtual void            GetTrianglesStart(GetTrianglesContext& context, const AABox& box, const Vec3& positionCOM, const Quat& rotation, const Vec3& scale) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetTrianglesNext()
        //----------------------------------------------------------------------------------------------------
        virtual int             GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested, Float3* outTriangleVertices) const override;

        //----------------------------------------------------------------------------------------------------
        /// @see : Shape::GetSubSShapeIDBitsRecursive().
        //----------------------------------------------------------------------------------------------------
        virtual unsigned        GetSubSShapeIDBitsRecursive() const override    { return 0; } // Convex Shapes don't have sub shapes. 

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the density of the shape (kg / m^3)
        //----------------------------------------------------------------------------------------------------
        void                    SetDensity(const float density)                 { m_density = density; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the density of the shape (kg / m^3)
        //----------------------------------------------------------------------------------------------------
        float                   GetDensity() const                              { return m_density; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Register shape functions within the registry. 
        //----------------------------------------------------------------------------------------------------
        static void             Register();

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function called by the CollisionSolver
        //----------------------------------------------------------------------------------------------------
        static void             CollideConvexVsConvex(const Shape* pShape1, const Shape* pShape2, const Vec3& scale1, const Vec3& scale2, const Mat44& centerOfMassTransform1, const Mat44& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, const CollideShapeSettings& collideShapeSettings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function called by the CollisionSolver
        //----------------------------------------------------------------------------------------------------
        static void			    CastConvexVsConvex(const ShapeCast& shapeCast, const ShapeCastSettings& shapeCastSettings, const Shape* pShape, const Vec3& scale, const ShapeFilter& shapeFilter, const Mat44& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, CastShapeCollector& collector);

    protected:
        static const StaticArray<Vec3, 384> s_unitSphereTriangles;

    private:
        //ConstStrongPtr<PhysicsMaterial> m_pMaterial;      /// Material assigned to this shape.
        float m_density = 1000.f;                           /// Uniform density of the interior of the convex object (kg / m^3)
    };
}
