// Shape.h
#pragma once
#include <cstdint>
#include "Nessie/Core/Result.h"
#include "Nessie/Core/StaticArray.h"
#include "Nessie/Core/Color.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Geometry/AABox.h"
#include "Nessie/Math/Scalar3.h"
#include "Nessie/Physics/Body/MassProperties.h"
#include "Nessie/Physics/Collision/BackFaceMode.h"
#include "Nessie/Physics/Collision/CollisionCollector.h"
#include "Nessie/Physics/Collision/ShapeFilter.h"

namespace nes
{
    class Shape;
    struct RayCast;
    struct RayCastSettings;
    struct RayCastResult;
    struct ShapeCast;
    struct ShapeCastSettings;
    struct ShapeCastResult;
    struct CollidePointResult;
    struct CollideShapeResult;
    class SubShapeIDCreator;
    class SubShapeID;
    class TransformedShape;
    // [TODO]: Declaration conflict.
    //class Plane;
    
    using CastRayCollector = CollisionCollector<RayCastResult, CollisionCollectorTraitsCastRay>;
    using CastShapeCollector = CollisionCollector<ShapeCastResult, CollisionCollectorTraitsCastShape>;
    using CollidePointCollector = CollisionCollector<CollidePointResult, CollisionCollectorTraitsCollidePoint>;
    using CollideShapeCollector = CollisionCollector<CollideShapeResult, CollisionCollectorTraitsCollideShape>;
    using TransformedShapeCollector = CollisionCollector<TransformedShape, CollisionCollectorTraitsCollideShape>;
    
    enum class EShapeType : uint8_t
    {
        Convex,
        Compound,
        Decorated,
        Mesh,
        HeightField,
        SoftBody,
        Plane,
        Empty,

        // [TODO]: User defined values
    };

    enum class EShapeSubType : uint8_t
    {
        // Convex Shapes
        Sphere,
        Box,
        Triangle,
        Capsule,
        TaperedCapsule,
        Cylinder,
        ConvexHull,

        // Compound Shapes
        StaticCompound,
        MutableCompound,

        // Decorated Shapes
        RotatedTranslated,
        Scaled,
        OffsetCenterOfMass,

        // Other
        Mesh,
        HeightField,
        SoftBody,
        Plane,
        TaperedCylinder,
        Empty,

        // [TODO]: User defined values
    };

    static constexpr EShapeSubType  kAllSubShapeTypes[] = { EShapeSubType::Sphere, EShapeSubType::Box, EShapeSubType::Triangle, EShapeSubType::Capsule, EShapeSubType::TaperedCapsule, EShapeSubType::Cylinder, EShapeSubType::ConvexHull, EShapeSubType::StaticCompound, EShapeSubType::MutableCompound, EShapeSubType::RotatedTranslated, EShapeSubType::Scaled, EShapeSubType::OffsetCenterOfMass, EShapeSubType::Mesh, EShapeSubType::HeightField, EShapeSubType::SoftBody, EShapeSubType::Plane, EShapeSubType::TaperedCylinder, EShapeSubType::Empty };
    static constexpr EShapeSubType  kConvexSubShapeTypes[] = { EShapeSubType::Sphere, EShapeSubType::Box, EShapeSubType::Triangle, EShapeSubType::Capsule, EShapeSubType::TaperedCapsule, EShapeSubType::Cylinder, EShapeSubType::ConvexHull, EShapeSubType::TaperedCylinder};
    static constexpr EShapeSubType  kCompoundSubShapeTypes[] = { EShapeSubType::StaticCompound, EShapeSubType::MutableCompound };
    static constexpr EShapeSubType  kDecoratorSubShapeTypes[] = { EShapeSubType::RotatedTranslated, EShapeSubType::Scaled, EShapeSubType::OffsetCenterOfMass };

    static constexpr unsigned       kNumSubShapeTypes = static_cast<unsigned>(std::size(kAllSubShapeTypes));
    static constexpr const char *   kSubShapeTypeNames[] = { "Sphere", "Box", "Triangle", "Capsule", "TaperedCapsule", "Cylinder", "ConvexHull", "StaticCompound", "MutableCompound", "RotatedTranslated", "Scaled", "OffsetCenterOfMass", "Mesh", "HeightField", "SoftBody", "Plane", "TaperedCylinder", "Empty" };
    static_assert(std::size(kSubShapeTypeNames) == kNumSubShapeTypes);

    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  This is intended to be a serializable object, and store shape data in 'uncooked' form (i.e. in
    //  a form that is still human-readable and authorable).
    //		
    /// @brief : Class that can construct shapes. 
    //----------------------------------------------------------------------------------------------------
    class ShapeSettings : public RefTarget<ShapeSettings>
    {
    public:
        using ShapeResult = Result<StrongPtr<Shape>>;

        /// User data (to be used freely by the application).
        uint64_t m_userData;
        
    public:
        virtual             ~ShapeSettings() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a shape according to the settings specified by this object. 
        //----------------------------------------------------------------------------------------------------
        virtual ShapeResult Create() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : When creating a shape, the result is cached so that calling Create() again will return the
        ///     same shape. If you make changes to the ShapeSettings you need to call this function to clear the
        ///     cached result to allow Create() to build a new shape.
        //----------------------------------------------------------------------------------------------------
        void                ClearCachedResult();

    protected:
        /// Cached result from the Create() function.
        mutable ShapeResult m_cachedResult;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Function table for operations on shapes 
    //----------------------------------------------------------------------------------------------------
    class ShapeFunctions
    {
    public:
        /// Function to construct a Shape.
        Shape* (*m_construct)() = nullptr;

        /// Color of the shape when drawing.
        Color  m_color = Color::Black();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get an entry in the registry for a particular subtype.
        //----------------------------------------------------------------------------------------------------
        static inline ShapeFunctions& Get(EShapeSubType subType) { return s_registry[static_cast<size_t>(subType)]; }

    private:
        static ShapeFunctions s_registry[kNumSubShapeTypes];
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all shapes (collision volume of a body). Defines a virtual interface for
    ///     collision detection.
    //----------------------------------------------------------------------------------------------------
    class Shape : public RefTarget<Shape>
    {
    public:
        using SupportingFace = StaticArray<Vec3, 32>;
        using ShapeResult = ShapeSettings::ShapeResult;

        //----------------------------------------------------------------------------------------------------
        /// @brief : An opaque buffer that holds shape specific information during GetTrianglesStart/Next. 
        //----------------------------------------------------------------------------------------------------
        struct alignas(16) GetTrianglesContext { uint8_t m_data[4288]; };

        /// This is the minimum amount of triangles that should be requested through GetTrianglesNext().
        static constexpr int kGetTrianglesMinTrianglesRequested = 32;
        
    public:
        Shape(const EShapeType type, const EShapeSubType subType) : m_shapeType(type), m_subShapeType(subType) {}
        Shape(const EShapeType type, const EShapeSubType subType, const ShapeSettings& settings, [[maybe_unused]] ShapeResult &outResult) : m_userData(settings.m_userData), m_shapeType(type), m_subShapeType(subType) {}
        Shape(const Shape&) = delete;
        Shape(Shape&&) noexcept = default;
        Shape& operator=(const Shape&) = delete;
        Shape& operator=(Shape&&) noexcept = default;
        virtual ~Shape() override = default;

    public:
        inline EShapeType       GetType() const                                 { return m_shapeType; }
        inline EShapeSubType    GetSubType() const                              { return m_subShapeType; }
        uint64_t                GetUserData() const                             { return m_userData; }
        void                    SetUserData(const uint64_t userData)            { m_userData = userData; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this shape can only be used to create a static body or if it can also be dynamic/
        ///     kinematic.
        //----------------------------------------------------------------------------------------------------
        virtual bool            MustBeStatic() const                            { return false; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : All shapes are centered around their center of mass (COM). This function returns the center
        ///     of mass position that needs to be applied to transform the shape to where it was created.
        //----------------------------------------------------------------------------------------------------
        virtual Vec3            GetCenterOfMass() const                         { return Vec3::Zero(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the local bounding box including the convex radius. This box is centered around the
        ///     center of mass rather than the world transform.
        //----------------------------------------------------------------------------------------------------
        virtual AABox           GetLocalBounds() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world space bounds including convex radius. This shape is scaled by scale in local
        ///     space first. This function can be overridden to return a closer fitting world space bounding
        ///     box; by default, it will just transform what GetLocalBounds() returns.
        //----------------------------------------------------------------------------------------------------
        virtual AABox           GetWorldBounds(const Mat44& centerOfMassTransform, const Vec3& scale) const;
        
        //----------------------------------------------------------------------------------------------------
        // ?
        /// @brief : Get the max number of sub shape ID bits that are need to be able to address any leaf shape
        ///     in this shape. Used mainly for checking that it is smaller or equal to SubShapeID::kMaxBits.
        //----------------------------------------------------------------------------------------------------
        virtual unsigned        GetSubSShapeIDBitsRecursive() const = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the radius of the biggest sphere that fits entirely in the shape. In case this
        ///     shape consists of multiple sub shapes, it returns the smallest sphere of the parts.
        ///     This can be used as a measure of how far the shape can be moved without risking going through
        ///     geometry.
        //----------------------------------------------------------------------------------------------------
        virtual float           GetInnerRadius() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the mass and inertia of this shape.
        //----------------------------------------------------------------------------------------------------
        virtual MassProperties  GetMassProperties() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the leaf shape for a paritcular sub shape ID. 
        ///	@param subShapeID : The full sub shape ID that indicates the path to the leaf shape.
        ///	@param outRemainder : What remains of the sub shape ID after removing the path to the leaf shape
        ///     (could e.g. refer to the triangle within a MeshShape).
        ///	@returns : The Shape or null if the sub shape ID is invalid.
        //----------------------------------------------------------------------------------------------------
        virtual const Shape*    GetLeafShape([[maybe_unused]] const SubShapeID& subShapeID, SubShapeID& outRemainder) const;

        // [TODO]: virtual const PhysicsMaterial* GetMaterial(Const SubShapeID& id) const = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the surface normal of a particular sub shape ID and point on the surface (All vectors are
        ///     relative to the center of mass of this shape).
        /// @note : When you have a CollideShapeResult or ShapeCastResult you should use -m_penetrationAxis.Normalized() as
        ///     a contact normal as GetSurfaceNormal() will only return the face normal (not the vertex or edge normals).
        //----------------------------------------------------------------------------------------------------
        virtual Vec3            GetSurfaceNormal(const SubShapeID& subShapeID, const Vec3& localSurfacePosition) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vertices of the face that faces 'direction' the most (includes any convex radius).
        ///     Note that this function can only return faces of convex shapes and triangles, which is why
        ///     a sub shape ID to get to that leaf is required.
        ///	@param subShapeID : ID of the target sub shape.
        ///	@param direction : Direction that the face should be facing (in local space to this shape).
        ///	@param scale : Scale in local space of the shape (scales relative to its center of mass).
        ///	@param centerOfMassTransform : Transform to transform the outVertices by.
        ///	@param outVertices : The resulting face. The returned face can be empty if the shape doesn't have
        ///     polygons to return (e.g. because it's a sphere). The face will be returned in world space. 
        //----------------------------------------------------------------------------------------------------
        virtual void            GetSupportingFace([[maybe_unused]] const SubShapeID& subShapeID, [[maybe_unused]] const Vec3& direction, [[maybe_unused]] const Vec3& scale, [[maybe_unused]] const Mat44& centerOfMassTransform, [[maybe_unused]] SupportingFace& outVertices) const { /* Nothing */ }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the user data of a particular sub shape ID. Corresponds to the value stored in Shape::GetUserData()
        ///     of the leaf shape pointed to by subShapeID.
        //----------------------------------------------------------------------------------------------------
        virtual uint64_t        GetSubShapeUserData([[maybe_unused]] const SubShapeID& subShapeID) const   { return m_userData; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the direct child sub shape and its transform for a sub shape ID.
        ///	@param subShapeID : Sub shape ID that indicates the path to the leaf shape.
        ///	@param positionCOM : The position of the center of mass of this shape.
        ///	@param rotation : The orientation of this shape. 
        ///	@param scale : Scale in local space of the shape (scales relative to its center of mass).
        ///	@param outRemainder : The remainder of the sub shape ID after removing the sub shape.
        ///	@returns : Direct child sub shape and its transform, not that the body ID and sub shape ID will be invalid.
        //----------------------------------------------------------------------------------------------------
        virtual TransformedShape GetSubShapeTransformedShape(const SubShapeID& subShapeID, const Vec3& positionCOM, const Quat& rotation, const Vec3& scale, SubShapeID& outRemainder) const;

        // [TODO]: Submerged Volume

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a ray against this shape. Returns true if it finds a hit closer than hitResult.m_fraction and
        ///     updates that fraction. Otherwise, the hitResult is left untouched and the function returns false.
        /// @note : The ray should be relative to the center of mass of this shape (i.e. subtract Shape::GetCenterOfMass()
        ///     from RayCast::m_origin if you want to cast against the shape in the space it was created).
        /// @note : Convex objects will be treated as solid (meaning if the ray starts inside, you'll get a hit fraction
        ///     of 0) and back face hits against triangles are returned.
        /// @note : If you want the surface normal of the hit use:
        ///     GetSurfaceNormal(hitResult.m_subShapeID2, ray.GetPointOnRay(hitResult.m_fraction)). 
        //----------------------------------------------------------------------------------------------------
        virtual bool            CastRay(const RayCast& ray, const SubShapeIDCreator& subShapeIDCreator, RayCastResult& hitResult) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a ray against this shape. Allows returning multiple hits through the 'collector'.
        /// @note : This version is more flexible but also slightly slower than the CastRay function that returns only a single hit.
        /// @note : If you want the surface normal of the hit use:
        ///     GetSurfaceNormal(hitResult.m_subShapeID2, ray.GetPointOnRay(hitResult.m_fraction)). 
        //----------------------------------------------------------------------------------------------------
        virtual void            CastRay(const RayCast& ray, const RayCastSettings& settings, const SubShapeIDCreator& subShapeIDCreator, CastRayCollector& collector, const ShapeFilter& shapeFilter = {}) const = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if 'point' is inside the shape. For this test all shapes are treated as if they
        ///     were solid. For each shape that collides, collector will receive a hit.
        /// @note : 'point' should be relative to the center of mass of this shape (i.e. subtract Shape::GetCenterOfMass()
        ///     from 'position' if you want to test against the shape in the space it was created).
        /// @note : For a mesh shape, this test will only provide sensible information if the mesh is a closed manifold.
        //----------------------------------------------------------------------------------------------------
        virtual void            CollidePoint(const Vec3& point, const SubShapeIDCreator& subShapeIDCreator, CollidePointCollector& collector, const ShapeFilter& shapeFilter = {}) const = 0;

        // [TODO]: Soft Body Collision
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Collect the leaf transformed shapes of all leaf shapes of this shape.
        ///	@param box : World space axis aligned box which leaf shapes should collide with.
        ///	@param positionCOM : Center of Mass Position of the shape transform.
        ///	@param rotation : Rotation of the shape transform.
        ///	@param scale : Scale of the shape transform.
        ///	@param subShapeIDCreator : Represents the current sub shape ID of this shape.
        ///	@param collector : Collector that stores all the transformed shapes.
        ///	@param shapeFilter : Filter to determine if this shape should collide with the current sub shape.
        //----------------------------------------------------------------------------------------------------
        virtual void            CollectTransformedShapes(const AABox& box, const Vec3& positionCOM, const Quat& rotation, const Vec3& scale, const SubShapeIDCreator& subShapeIDCreator, TransformedShapeCollector& collector, const ShapeFilter& shapeFilter) const;
        
        //----------------------------------------------------------------------------------------------------
        //	NOTES:
        //		
        /// @brief : Transforms this shape and all of its children with the transform, resulting shape(s) are
        ///     passed to the 'collector'.
        /// @note : Not all shapes support all transforms (especially true for scaling), the resulting shape will
        ///     try to match the transform as accurately as possible.
        ///	@param centerOfMassTransform : The transform (rotation, translation, scale) and the center of mass of the shape
        ///     should be set to.
        ///	@param collector : The transformed shapes will be passed to this collector.
        //----------------------------------------------------------------------------------------------------
        virtual void            TransformShape(const Mat44& centerOfMassTransform, TransformedShapeCollector& collector) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale this shape.
        /// @note : Not all shapes support all scales, this will return a shape that matches the scale as accurately
        ///     as possible. See Shape::IsValidScale() for more information.
        ///	@param scale : Scale to apply to this shape. This scale is applied to the entire shape in the space
        ///     it was created - most other functions apply the scale in the space of the leaf shapes and from
        ///     the center of mass!
        //----------------------------------------------------------------------------------------------------
        ShapeResult             ScaleShape(const Vec3& scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : To start iterating over triangles, call this function first.
        ///	@param context : A temporary buffer and should remain untouched until the last call to GetTrianglesNext().
        ///	@param box : World space bounding box in which you want to get the triangles.
        ///	@param positionCOM : Describes the position of the Shape.
        ///	@param rotation : Describes the rotation of the shape.
        ///	@param scale : Describes the scale of the shape.
        //----------------------------------------------------------------------------------------------------
        virtual void            GetTrianglesStart(GetTrianglesContext& context, const AABox& box, const Vec3& positionCOM, const Quat& rotation, const Vec3& scale) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call this repeatedly to get all triangles in the box.
        ///     'outTriangleVertices' should be large enough to hold 3 * maxTrianglesRequested entries.
        ///     The function returns the amount of triangles that it found (which will be <= maxTrianglesRequested), or
        ///     0 if there are no more triangles.
        /// @note : The function can return a value < maxTrianglesRequested and still have more triangles to process (triangles can be returned in blocks).
        /// @note : The function may return triangles outside the requested box, only course culling is performed on the returned
        ///     triangles.
        //----------------------------------------------------------------------------------------------------
        virtual int             GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested, Float3* outTriangleVertices/*, const PhyscisMaterial** outMaterials = nullptr*/ ) const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the volume of this Shape (m^3).
        /// @note : For compound shapes the volume may be incorrect since child shapes can overlap which is not
        ///     accounted for.
        //----------------------------------------------------------------------------------------------------
        virtual float           GetVolume() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if inScale is a valid scale for this shape. Some shapes can only be scaled uniformly,
        ///     compound shapes cannot handle shapes being rotated and scaled (this would cause shearing),
        ///     scale can never be zero. When the scale is invalid, the function will return false.
        ///
        /// Here's a list of supported scales:
        ///     * SphereShape: Scale must be uniform (signs of scale are ignored).
        ///     * BoxShape: Any scale supported (signs of scale are ignored).
        ///     * TriangleShape: Any scale supported when convex radius is zero, otherwise only uniform scale supported.
        ///     * CapsuleShape: Scale must be uniform (signs of scale are ignored).
        ///     * TaperedCapsuleShape: Scale must be uniform (sign of Y scale can be used to flip the capsule).
        ///     * CylinderShape: Scale must be uniform in XZ plane, Y can scale independently (signs of scale are ignored).
        ///     * RotatedTranslatedShape: Scale must not cause shear in the child shape.
        ///     * CompoundShape: Scale must not cause shear in any of the child shapes.
        //----------------------------------------------------------------------------------------------------
        virtual bool            IsValidScale(const Vec3& scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function will make sure that if you wrap this shape in a ScaledShape that the scale
        ///     will be valid.
        /// @note : This involves discarding components of the scale that are invalid, so the resulting scaled
        ///     shape may be different from the requested scale. Compare the return value of this function with
        ///     the scale you passed in to detect major inconsistencies and possibly warn the user.
        ///	@param scale : The local space scale for this shape. 
        ///	@returns : Scale that can be used to wrap this shape in a ScaledShape. IsValidScale() will return true for this scale.
        //----------------------------------------------------------------------------------------------------
        virtual Vec3            MakeScaleValid(const Vec3& scale) const;

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : A fallback version of CollidePoint() that uses a ray cast and counts the number of hits to
        ///     determine if the point is inside the shape or not. Odd number of hits means inside, even number of hits means outside.
        //----------------------------------------------------------------------------------------------------
        static void             CollidePointUsingRayCast(const Shape& shape, const Vec3& point, const SubShapeIDCreator& subShapeIDCreator, CollidePointCollector& collector, const ShapeFilter& shapeFilter);

    private:
        uint64_t                m_userData = 0;
        EShapeType              m_shapeType;
        EShapeSubType           m_subShapeType;
    };
}
