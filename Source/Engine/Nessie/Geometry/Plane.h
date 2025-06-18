// Plane.h
#pragma once
#include "Math/Vec4.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Plane, stored in constant-normal form.
    ///     When the normal is of unit length, the constant represents the signed distance from the origin
    ///     in the direction of the normal. Otherwise, it still represents the distance but in units of the
    ///     normal's length.  
    //----------------------------------------------------------------------------------------------------
    class Plane
    {
    public:
        Plane() = default;
        explicit    Plane(const Vec4& normalAndConstant);
        Plane(const Vec3 normal, const float constant);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a plane from a point and normal. 
        //----------------------------------------------------------------------------------------------------
        static inline Plane FromPointAndNormal(const Vec3 point, const Vec3 normal)     { return Plane(Vec4(normal, -normal.Dot(point))); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a plane from 3 counter-clockwise points.
        //----------------------------------------------------------------------------------------------------
        static inline Plane FromPointsCCW(const Vec3 v1, const Vec3 v2, const Vec3 v3)  { return FromPointAndNormal(v1, (v2 - v1).Cross(v3 - v1).Normalized()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the plane normal. 
        //----------------------------------------------------------------------------------------------------
        inline Vec3         GetNormal() const                                           { return Vec3(m_normalAndConstant); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the plane normal. 
        //----------------------------------------------------------------------------------------------------
        inline void         SetNormal(const Vec3 normal)                                { m_normalAndConstant = Vec4(normal, m_normalAndConstant.w); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the constant.
        ///     When the normal is of unit length, the constant represents the signed distance from the origin
        ///     in the direction of the normal. Otherwise, it still represents the distance but in units of the
        ///     normal's length.  
        //----------------------------------------------------------------------------------------------------
        inline float        GetConstant() const                                         { return m_normalAndConstant.w; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the constant.
        ///     When the normal is of unit length, the constant represents the signed distance from the origin
        ///     in the direction of the normal. Otherwise, it still represents the distance but in units of the
        ///     normal's length.   
        //----------------------------------------------------------------------------------------------------
        inline void         SetConstant(const float constant)                           { m_normalAndConstant.w = constant; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns this plane offset by a distance. A positive value means to move it in the direction of the plane normal.
        //----------------------------------------------------------------------------------------------------
        inline Plane        Offset(const float distance) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform a plane by a matrix.
        //----------------------------------------------------------------------------------------------------
        inline Plane        Transformed(const Mat44& transform) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale the plane. This can handle non-uniform and negative scaling.
        //----------------------------------------------------------------------------------------------------
        inline Plane        Scaled(const Vec3 scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the distance of a point to the plane. If the result is negative, then the point is
        ///     behind the plane. If positive, the point is in front. If equal to zero, then the point is on
        ///     the plane and considered coplanar.
        ///     This can also be thought of as the Plane's Dot product. 
        //----------------------------------------------------------------------------------------------------
        inline float        SignedDistanceTo(const Vec3 point) const                      { return point.Dot(GetNormal()) + GetConstant(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the projected point on the plane - also the closest point on the plane to the point.
        //----------------------------------------------------------------------------------------------------
        inline Vec3         ProjectPointOnPlane(const Vec3 point) const                 { return point - GetNormal() * SignedDistanceTo(point); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the point is on the plane.
        //----------------------------------------------------------------------------------------------------
        inline bool         IsOnPlane(const Vec3 point) const                           { return SignedDistanceTo(point) == 0.f; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the intersection point between 3 planes. Returns false if there is no single
        ///     intersection point.
        //----------------------------------------------------------------------------------------------------
        static inline bool  IntersectPlanes(const Plane& plane1, const Plane& plane2, const Plane& plane3, Vec3& outPoint);
        
    private:
        Vec4                m_normalAndConstant; /// XYZ = normal, W = constant. Point X is on the plane: x.Dot(normal) + constant = 0.
    };

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Three-dimensional Plane.  
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // struct TPlane
    // {
    //     // Plane Normal. Any point "X" that is on the plane must satisfy Dot(m_normal, X) = m_distance;
    //     TVector3<Type> m_normal = TVector3<Type>::Up();
    //     
    //     // Distance of the Plane from the origin.
    //     Type m_distance = {};
    //
    //     constexpr TPlane() = default;
    //     TPlane(const TVector3<Type>& normal, Type distance);
    //     TPlane(const TVector3<Type>& normal, const TVector3<Type>& point);
    //     TPlane(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c);
    //
    //     constexpr bool operator==(const TPlane& other) const;
    //     constexpr bool operator!=(const TPlane& other) const { return !(*this == other); }
    //
    //     TVector3<Type> Origin() const;
    //     TVector3<Type> ClosestPointToPoint(const TVector3<Type>& point) const;
    //     Type SignedDistanceToPoint(const TVector3<Type>& point) const;
    //     bool IsOnPlane(const TVector3<Type>& point) const;
    // };
    //
    // using Planef = TPlane<float>;
    // using Planed = TPlane<double>;
    // using Plane = TPlane<NES_PRECISION_TYPE>;
}

namespace nes
{
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Construct a plane from a normal and a distance from the origin.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // TPlane<Type>::TPlane(const TVector3<Type>& normal, Type distance)
    //     : m_normal(normal)
    //     , m_distance(distance)
    // {
    //     m_normal.Normalize(); // ensure that the plane is normalized.
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Construct a Plane from a normal and a point on that normal.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // TPlane<Type>::TPlane(const TVector3<Type>& normal, const TVector3<Type>& point)
    // {
    //     m_normal = normal.Normalized(); // ensure normalization.
    //     m_distance = TVector3<Type>::Dot(m_normal, point);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Construct a plane from 3 non-collinear points (ordered counterclockwise).
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // TPlane<Type>::TPlane(const TVector3<Type>& a, const TVector3<Type>& b,
    //     const TVector3<Type>& c)
    // {
    //     m_normal = TVector3<Type>::Cross(b - a, c - a).Normalized();
    //     m_distance = TVector3<Type>::Dot(m_normal, a);
    // }
    //
    // template <FloatingPointType Type>
    // constexpr bool TPlane<Type>::operator==(const TPlane& other) const
    // {
    //     return m_normal == other.m_normal && math::CheckEqualFloats(m_distance, other.m_distance);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Get this Plane's origin. 
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // TVector3<Type> TPlane<Type>::Origin() const
    // {
    //     // m_distance represents a distance from the Origin, and m_normal is the direction from the Origin,
    //     // so the Plane's origin will be the m_distance away from the Origin in the direction of the normal.
    //     return m_normal * m_distance;
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Returns the closest point on the Plane to the query point. 
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // TVector3<Type> TPlane<Type>::ClosestPointToPoint(const TVector3<Type>& point) const
    // {
    //     const Type distance = TVector3<Type>::Dot(m_normal, point) - m_distance;
    //     return point - (m_normal * distance);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Returns the distance of a point to the plane. If the result is negative, then the point is
    // ///             behind the plane. If positive, the point is in front. If equal to zero, then the point is on
    // ///             the plane & considered coplanar.
    // ///             This can also be thought of as the Plane's Dot product.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // Type TPlane<Type>::SignedDistanceToPoint(const TVector3<Type>& point) const
    // {
    //     return TVector3<Type>::Dot(m_normal, point) - m_distance;
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Returns true if the point lies on the plane's surface. This is the same as checking if
    // ///             SignedDistanceToPoint is equal to zero.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // bool TPlane<Type>::IsOnPlane(const TVector3<Type>& point) const
    // {
    //     return math::CheckEqualFloats(SignedDistanceToPoint(point), static_cast<Type>(0));
    // }
}
