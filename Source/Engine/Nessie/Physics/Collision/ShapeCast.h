// ShapeCast.h
#pragma once

#include "Geometry/AABox.h"
#include "CollideShape.h"
#include "Shapes/Shape.h"

namespace nes
{
    struct ShapeCast
    {
        const Shape*    m_pShape;               /// Shape that's being cast (cannot be a mesh shape). Note that this structure does not assume ownership over the shape for performance reasons.
        const Vec3      m_scale;                /// Scale in local space of the shape being cast (scales relative to its center of mass).
        const Mat44     m_centerOfMassStart;    /// Start position and orientation of the center of mass of the shape (construct using FromWorldTransform() if you have a world transform for your shape).
        const Vec3      m_direction;            /// Direction and length of the cast (anything beyond this length will not be reported as a hit).
        const AABox     m_shapeWorldBounds;     /// Cached shape's world bounds, calculated in constructor.

        ShapeCast(const Shape* pShape, const Vec3& scale, const Mat44& centerOfMassStart, const Vec3& direction, const AABox& shapeWorldBounds)
            : m_pShape(pShape)
            , m_scale(scale)
            , m_centerOfMassStart(centerOfMassStart)
            , m_direction(direction)
            , m_shapeWorldBounds(shapeWorldBounds)
        {
            //
        }

        ShapeCast(const Shape* pShape, const Vec3& scale, const Mat44& centerOfMassStart, const Vec3& direction)
            : ShapeCast(pShape, scale, centerOfMassStart, direction, pShape->GetWorldBounds(centerOfMassStart, scale))
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a shape cast using a world transform for a shape instead of the center of mass transform. 
        //----------------------------------------------------------------------------------------------------
        static inline ShapeCast FromWorldTransform(const Shape* pShape, const Vec3& scale, const Mat44& worldTransform, const Vec3& direction)
        {
            return ShapeCast(pShape, scale, worldTransform.PreTranslated(pShape->GetCenterOfMass()), direction);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a transformed copy of this shape cast using 'transform'. Multiply transform on the
        ///     left hand side.
        //----------------------------------------------------------------------------------------------------
        ShapeCast               PostTransformed(const Mat44& transform) const
        {
            Mat44 start = transform * m_centerOfMassStart;
            Vec3 direction = transform.TransformVector(m_direction);
            return { m_pShape, m_scale, start, direction };
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a translated copy of this shape cast by 'translation'. 
        //----------------------------------------------------------------------------------------------------
        ShapeCast               PostTranslated(const Vec3& translation) const
        {
            return { m_pShape, m_scale, m_centerOfMassStart.PostTranslated(translation), m_direction };
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a point that is 'distance' along the ray form m_centerOfMassStart to m_centerOfMassStart + m_direction * fraction
        ///     where 0 = start of the ray and 1 = end of the ray.
        //----------------------------------------------------------------------------------------------------
        Vec3                 GetPointAlongRay(const float fraction) const
        {
            return m_centerOfMassStart.GetColumn3(3) + (fraction * m_direction); 
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings to be passed with a shape cast. 
    //----------------------------------------------------------------------------------------------------
    struct ShapeCastSettings : public CollideShapeSettingsBase
    {
        /// How backfacing triangles should be treated. Should we report back facing hits for triangle based shapes, e.g.
        /// MeshShape/HeightFieldShape?
        EBackFaceMode   m_backfaceModeTriangles    = EBackFaceMode::IgnoreBackFaces;

        /// How back facing convex objects should be treated. Should we report back facing hits on convex shapes?
        EBackFaceMode   m_backfaceModeConvex       = EBackFaceMode::IgnoreBackFaces;

        /// Indicates if we want to shrink the shape by the convex radius and then expand it again. This speeds up collision
        /// detection and gives a more accurate normal at the cost of a more 'rounded' shape.
        bool            m_useShrunkenShapeAndConvexRadius = false;

        /// When true, and the shape is intersecting at the beginning of the cast (fraction = 0), then this will calculate
        /// the deepest penetration point (costing additional CPU time).
        bool            m_returnDeepestPoint = false;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the back face mode for all shapes. 
        //----------------------------------------------------------------------------------------------------
        void SetBackFaceMode(const EBackFaceMode& backfaceMode) { m_backfaceModeTriangles = m_backfaceModeConvex = backfaceMode; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Result of a Shape Cast Test. 
    //----------------------------------------------------------------------------------------------------
    struct ShapeCastResult : public CollideShapeResult
    {
        float   m_fraction;         /// This is the fraction where the shape hit the other shape: CenterOfMassHit = Start + value * (End - Start).
        bool    m_isBackFaceHit;    /// True if the shape was hit from the back side.
        
        ShapeCastResult() = default;
        ShapeCastResult(const float fraction, const Vec3& contactPoint1, const Vec3& contactPoint2, const Vec3& contactNormalOrPenetrationDepth, bool isBackFaceHit, const SubShapeID& subShapeID1, const SubShapeID& subShapeID2, const BodyID& bodyID2)
            : CollideShapeResult(contactPoint1, contactPoint2, contactNormalOrPenetrationDepth, (contactPoint2 - contactPoint1).Length(), subShapeID1, subShapeID2, bodyID2)
            , m_fraction(fraction)
            , m_isBackFaceHit(isBackFaceHit)
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function required by the Collision Collector. A smaller fraction is considered to be
        ///     a 'better hit'. For rays/cast shapes we can just use the collision fraction.
        ///     The fraction and penetration depth are combined in such a way that deeper hits at fraction 0 go first.
        //----------------------------------------------------------------------------------------------------
        inline float    GetEarlyOutFraction() const { return m_fraction > 0.f? m_fraction : -m_penetrationDepth;}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reverses the hit result, swapping contact point 1 with contact point 2, etc.
        ///	@param worldSpaceCastDirection : Direction of the shape cast in world space.
        //----------------------------------------------------------------------------------------------------
        ShapeCastResult Reversed(const Vec3& worldSpaceCastDirection) const
        {
            Vec3 delta = m_fraction * worldSpaceCastDirection;

            ShapeCastResult result;
            result.m_contactPointOn2 = m_contactPointOn1 - delta;
            result.m_contactPointOn1 = m_contactPointOn2 - delta;
            result.m_penetrationAxis = -m_penetrationAxis;
            result.m_penetrationDepth = m_penetrationDepth;
            result.m_subShapeID2 = m_subShapeID1;
            result.m_subShapeID1 = m_subShapeID2;
            result.m_bodyID2 = m_bodyID2;
            result.m_fraction = m_fraction;
            result.m_isBackFaceHit = m_isBackFaceHit;

            result.m_shape2Face.resize(m_shape1Face.size());
            for (Face::size_type i = 0; i < m_shape1Face.size(); ++i)
            {
                result.m_shape2Face[i] = m_shape1Face[i] - delta;
            }

            result.m_shape1Face.resize(m_shape2Face.size());
            for (Face::size_type i = 0; i < m_shape2Face.size(); ++i)
            {
                result.m_shape1Face[i] = m_shape2Face[i] - delta;
            }
            
            return result;
        }
    };
}