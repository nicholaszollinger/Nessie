// TransformedShape.h
#pragma once
#include "BackFaceMode.h"
#include "CollisionLayer.h"
#include "ShapeFilter.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/StrongPtr.h"
#include "Math/Float3.h"
#include "Math/Matrix.h"
#include "Physics/Body/BodyID.h"
#include "Shapes/Shape.h"
#include "Shapes/SubShapeID.h"

namespace nes
{
    struct ShapeCast;
    struct CollideShapeSettings;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A temporary data structure that contains a shape and a transform.
    ///     This structure can be obtained from a body (e.g. after a broad phase query) under lock protection.
    ///     The lock can then be release and collision detection operations can be safely performed since the
    ///     class takes a reference on teh shape and does not use anything from the body anymore.
    //----------------------------------------------------------------------------------------------------
    class TransformedShape
    {
    public:
        using GetTrianglesContext = Shape::GetTrianglesContext;
        
        Vector3                 m_shapePositionCOM;
        Quat                    m_shapeRotation;
        ConstStrongPtr<Shape>   m_pShape;
        Float3                  m_shapeScale { 1.0, 1.0, 1.0 };
        BodyID                  m_bodyID;
        SubShapeIDCreator       m_subShapeIDCreator;
        
    public:
        NES_OVERRIDE_NEW_DELETE

        TransformedShape() = default;
        inline TransformedShape(const Vector3& positionCOM, const Quat& rotation, const Shape* pShape, const BodyID& bodyID, const SubShapeIDCreator& subShapeIdCreator = SubShapeIDCreator());

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a ray and find the closest hit. Returns true if it finds a hit. Hits further than
        ///     hit.m_fraction will not be considered and in this case hit will remain unmodified (and the
        ///     function will return false). Convex objects will be treated as solid (meaning that if the
        ///     ray starts inside, you'll get a hit fraction of 0) and back face hits are returned.
        ///     If you want the surface normal of the hit use:
        ///         <code> GetWorldSpaceSurfaceNormal(hit.m_subShapeID2, ray.GetPointOnRay(hit.m_fraction))</code>
        ///     on this object.
        //----------------------------------------------------------------------------------------------------
        bool                    CastRay(const RayCast& ray, RayCastResult& hit) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a ray, allows collecting multiple hits. Note that this version is more flexible but also
        ///     slightly slower than the CastRay function that only returns a single hit.
        ///     If you want the surface normal of the hit use:
        ///         <code> GetWorldSpaceSurfaceNormal(hit.m_subShapeID2, ray.GetPointOnRay(hit.m_fraction))</code>
        ///     on this object.
        //----------------------------------------------------------------------------------------------------
        void                    CastRay(const RayCast& ray, const RayCastSettings& rayCastSettings, CastRayCollector& collector, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks if the point is inside any shapes. For this test all shapes are treated as if they
        ///     were solid. For a mesh shape, this test will only provide sensible information if the mesh is a
        ///     closed manifold. For each shape that collides, 'collector' will receive a hit.
        //----------------------------------------------------------------------------------------------------
        void                    CollidePoint(const Vector3& point, CollidePointCollector& collector, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Collide a shape and report any hits in 'collector'.
        ///	@param pShape : Shape to test.
        ///	@param shapeScale : Scale in local space of the shape.
        ///	@param centerOfMassTransform : Center of mass transform for the shape.
        ///	@param collideShapeSettings : Settings.
        ///	@param baseOffset : All hit results will be returned relative to this offset; can be zero to get results
        ///     in world space.
        ///	@param collector : Collector that receives the hits.
        ///	@param shapeFilter : Filter that allows you to reject certain collisions.
        //----------------------------------------------------------------------------------------------------
        void                    CollideShape(const Shape* pShape, const Vector3& shapeScale, const Mat4& centerOfMassTransform, const CollideShapeSettings& collideShapeSettings, const Vector3& baseOffset, CollideShapeCollector& collector, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast a shape and report any hits in the collector.
        ///	@param shapeCast : The shape cast with its position and direction.
        ///	@param settings : Settings for the shape cast.
        ///	@param baseOffset : All hit results will be returned relative to this offset; can be zero to get results
        ///     in world space.
        ///	@param collector : Collector that receives the hits.
        ///	@param shapeFilter : Filter that allows you to reject collisions.
        //----------------------------------------------------------------------------------------------------
        void                    CastShape(const ShapeCast& shapeCast, const ShapeCastSettings& settings, const Vector3& baseOffset, CastShapeCollector& collector, const ShapeFilter& shapeFilter = {}) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Collect the leaf transformed shapes of all leaf shapes of this shape. 'box' is the world
        ///     space axis aligned box which leaf shapes should collide with.
        //----------------------------------------------------------------------------------------------------
        void                    CollectTransformedShapes(const AABox& box, TransformedShapeCollector& collector, const ShapeFilter& shapeFilter = {}) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : To start iterating over triangles, call this function first.
        ///     To get the actual triangles, call GetTrianglesNext().
        ///	@param context : A temporary buffer and should remain untouched until the last call to GetTrianglesNext().
        ///	@param box : The world space bounding box in which you want to get the triangles.
        ///	@param baseOffset : All hit results will be returned relative to this offset; can be zero to get results
        ///     in world space.
        //----------------------------------------------------------------------------------------------------
        void                    GetTrianglesStart(GetTrianglesContext& context, const AABox& box, const Vector3& baseOffset) const;

        //----------------------------------------------------------------------------------------------------
        //	NOTES:
        //     "outMaterials (if it is not null) should contain 'maxTrianglesRequested' entries."
        //		
        /// @brief : Call this repeatedly to get all triangles in the box, after an initial call to GetTrianglesStart().
        ///     outTriangleVertices should be large enough to hold 3 * 'maxTrianglesRequested' entries.
        ///     The function returns the amount of triangles that it found (which will be <= maxTrianglesRequested)
        ///     or 0 if there are no more triangles.
        /// @note : The function can return a value < maxTrianglesRequested and still have more triangles to
        ///     process (triangles can be returned in blocks).
        /// @note : The function may return triangles outside the requested box, only course culling is performed
        ///     on the returned triangles.
        //----------------------------------------------------------------------------------------------------
        int                     GetTrianglesNext(GetTrianglesContext& context, int maxTrianglesRequested, Float3* outTriangleVertices/*, cont PhysicsMaterial** outMaterials*/) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the scale of the shape. 
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetShapeScale() const                   { return Vector3(m_shapeScale[0], m_shapeScale[1], m_shapeScale[2]); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the scale of the shape.  
        //----------------------------------------------------------------------------------------------------
        inline void             SetShapeScale(const Vector3& scale)     { m_shapeScale.x = scale.x; m_shapeScale.y = scale.y; m_shapeScale.z = scale.z; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the center of mass transform for this shape's center of mass (excluding scale). 
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetCenterOfMassTransform() const        { return math::MakeRotationTranslationMatrix(m_shapePositionCOM, m_shapeRotation); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the inverse of the center of mass transform for this shape's center of mass (excluding scale). 
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetInverseCenterOfMassTransform() const { return math::MakeInverseRotationTranslationMatrix(m_shapePositionCOM, m_shapeRotation); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world transform (including scale) of this transformed shape.
        /// @note : This is not from the center of mass, but in the space the shape was created.
        //----------------------------------------------------------------------------------------------------
        inline void             SetWorldTransform(const Vector3& position, const Quat& rotation, const Vector3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world transform (including scale) of this transformed shape.
        /// @note : This is not from the center of mass, but in the space the shape was created.
        //----------------------------------------------------------------------------------------------------
        inline void             SetWorldTransform(const Mat4& transform);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world transform (including scale) of this transformed shape.
        /// @note : This is not from the center of mass, but in the space the shape was created.
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetWorldTransform() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world space bounding box for the transformed shape. 
        //----------------------------------------------------------------------------------------------------
        inline AABox            GetWorldSpaceBounds() const                 { return m_pShape != nullptr? m_pShape->GetWorldBounds(GetCenterOfMassTransform(), GetShapeScale()) : AABox(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make a copy of 'subShapeID' that is relative to this shape. When the m_subShapeIDCreator is
        ///     not empty, this is needed in order to get the correct path to the subshape.
        //----------------------------------------------------------------------------------------------------
        inline SubShapeID       MakeSubShapeIDRelativeToShape(const SubShapeID& subShapeID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the surface normal of a particular sub shape and its world space surface position on
        ///     this Body.
        /// @note : When you have a CollideShapeResult or ShapeCastResult you should use -m_penetrationAxis.Normalize()
        ///     as contact normal as GetWorldSpaceSurfaceNormal() will only return face normals (and not vertex
        ///     or edge normals).
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetWorldSpaceSurfaceNormal(const SubShapeID& subShapeID, const Vector3& position) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vertices of the face that faces 'direction' the most (Includes any convex radius).
        /// @note : This function can only return faces of convex shapes or triangles, which is why a sub shape ID
        ///     to get to that leaf is provided.
        ///	@param subShapeID : Sub shape ID of target shape.
        ///	@param direction : Direction that the face should be facing (in world space).
        ///	@param baseOffset : The vertices will be returned relative to this offset. Can be zero to get results in
        ///     world space.
        ///	@param outVertices : Resulting face. Note that the returned face can have a single point if the shape
        ///     doesn't have polygons to return (e.g. because it's a sphere). The face will be returned in world space.
        //----------------------------------------------------------------------------------------------------
        inline void             GetSupportingFace(const SubShapeID& subShapeID, const Vector3& direction, const Vector3& baseOffset, Shape::SupportingFace& outVertices) const;

        // [TODO]: Physics Material

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the user data of a particular sub shape. 
        //----------------------------------------------------------------------------------------------------
        inline uint64_t         GetSubShapeUserData(const SubShapeID& subShapeID) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the direct child sub shape and its transform for a sub shape ID.
        ///	@param subShapeID : Sub shape ID that indicates the path to the leaf shape.
        ///	@param outRemainder : The remainder of the sub shape ID after removing the sub shape.
        ///	@returns : Direct child sub shape and its transform.
        /// @note : The body ID and sub shape ID will be invalid after this call.
        //----------------------------------------------------------------------------------------------------
        inline TransformedShape GetSubShapeTransformedShape(const SubShapeID& subShapeID, SubShapeID& outRemainder) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to return the body id from a transformed shape. If the transformed shape is
        ///     nullptr an invalid body id will be returned.
        //----------------------------------------------------------------------------------------------------
        inline static BodyID    GetBodyID(const TransformedShape* pShape)   { return pShape != nullptr? pShape->m_bodyID : BodyID(); }
    };
}

#include "TransformedShape.inl"