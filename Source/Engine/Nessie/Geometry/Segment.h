// Segment.h
#pragma once
#include "Math/Vec2.h"
#include "Math/Vec3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //  Unreal decided to store the segment as an origin, direction and extent (half-length).
    //  This adds an extra float, but would make certain operations faster, namely the Length & direction.
    //
    ///	@brief : A Line Segment between a start and end point, expressed in 2D coordinates. 
    //----------------------------------------------------------------------------------------------------
    struct Segment2
    {
        Vec2 m_start{};
        Vec2 m_end{};

        Segment2() = default;
        Segment2(const Vec2 start, const Vec2 end) : m_start(start), m_end(end) {}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the normalized direction from the start to the end. 
        //----------------------------------------------------------------------------------------------------
        Vec2    Direction() const                       { return (m_end - m_start).Normalized(); }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the vector displacement from the segment's start to the end. 
        //----------------------------------------------------------------------------------------------------
        Vec2    Vector() const                          { return m_end - m_start; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the length of the line segment. 
        //----------------------------------------------------------------------------------------------------
        float   Length() const                          { return (m_end - m_start).Length(); }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the squared length of the line segment.
        //----------------------------------------------------------------------------------------------------
        float   LengthSqr() const                       { return (m_end - m_start).LengthSqr(); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the center of the line segment, or midpoint. 
        //----------------------------------------------------------------------------------------------------
        Vec2    Center() const                          { return (m_start + m_end) * 0.5f; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the closest point on the Segment to the query point. 
        //----------------------------------------------------------------------------------------------------
        Vec2    ClosestPoint(const Vec2 queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the distance from the query point to the nearest point on the line segment.
        //----------------------------------------------------------------------------------------------------
        float   Distance(const Vec2 queryPoint) const   { return std::sqrt(DistanceSqr(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the squared distance from the query point to the nearest point on the line segment.
        //----------------------------------------------------------------------------------------------------
        float   DistanceSqr(const Vec2 queryPoint) const;
    };
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Line Segment between a start and end point, expressed in 3D coordinates. 
    //----------------------------------------------------------------------------------------------------
    struct Segment
    {
        Vec3 m_start{};
        Vec3 m_end{};

        Segment() = default;
        Segment(const Vec3 start, const Vec3 end) : m_start(start), m_end(end) {}

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the normalized direction from the start to the end. 
        //----------------------------------------------------------------------------------------------------
        Vec3    Direction() const                       { return (m_end - m_start).Normalized(); }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the vector displacement from the segment's start to the end. 
        //----------------------------------------------------------------------------------------------------
        Vec3    Vector() const                          { return m_end - m_start; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the length of the line segment. 
        //----------------------------------------------------------------------------------------------------
        float   Length() const                          { return (m_end - m_start).Length(); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the squared length of the line segment.
        //----------------------------------------------------------------------------------------------------
        float   LengthSqr() const                       { return (m_end - m_start).LengthSqr(); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the center of the line segment, or midpoint. 
        //----------------------------------------------------------------------------------------------------
        Vec3    Center() const                          { return (m_start + m_end) * 0.5f; }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the closest point on the Segment to the query point. 
        //----------------------------------------------------------------------------------------------------
        Vec3    ClosestPoint(const Vec3 queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the distance from the query point to the nearest point on the line segment.
        //----------------------------------------------------------------------------------------------------
        float   Distance(const Vec3 queryPoint) const   { return std::sqrt(DistanceSqr(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return the squared distance from the query point to the nearest point on the line segment.
        //----------------------------------------------------------------------------------------------------
        float   DistanceSqr(const Vec3 queryPoint) const;
    };

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Computes the closest points "closestOnA" and "closestOnB" between the segments, the normalized
    ///     parameters "tA" and "tB" that represent the position of the closest point to the respective segment where
    ///     0 == the start point and 1 == the end point. The return value is the squared distance between the
    ///     two closest points.
    ///	@param a : Segment A.
    ///	@param b : Segment B.
    ///	@param closestOnA : The closest point on Segment A to the Segment B.
    ///	@param closestOnB : The closest point on Segment B to the Segment A.
    /// @param tA : t value along Segment A that represents the position of the closest point from the
    ///     start point (0) to the end (1) of segment A.  
    /// @param tB : t value along Segment B that represents the position of the closest point from the
    ///     start point (0) to the end (1) of segment B.  
    ///	@returns : The Squared Distance between the two closest points.
    //----------------------------------------------------------------------------------------------------
    float ClosestPointsBetweenSegments(const Segment2& a, const Segment2& b, Vec2& closestOnA, Vec2& closestOnB, float tA, float tB);

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Computes the closest points "closestOnA" and "closestOnB" between the segments, the normalized
    ///     parameters "tA" and "tB" that represent the position of the closest point to the respective segment where
    ///     0 == the start point and 1 == the end point. The return value is the squared distance between the
    ///     two closest points.
    ///	@param a : Segment A.
    ///	@param b : Segment B.
    ///	@param closestOnA : The closest point on Segment A to the Segment B.
    ///	@param closestOnB : The closest point on Segment B to the Segment A.
    /// @param tA : t value along Segment A that represents the position of the closest point from the
    ///     start point (0) to the end (1) of segment A.  
    /// @param tB : t value along Segment B that represents the position of the closest point from the
    ///     start point (0) to the end (1) of segment B.  
    ///	@returns : The Squared Distance between the two closest points.
    //----------------------------------------------------------------------------------------------------
    float ClosestPointsBetweenSegments(const Segment& a, const Segment& b, Vec3& closestOnA, Vec3& closestOnB, float tA, float tB);
}