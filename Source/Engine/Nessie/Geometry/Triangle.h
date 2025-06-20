// Triangles.h
#pragma once
#include "Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Triangle class that takes 2D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    struct Triangle2
    {
        Float2              m_vertices[3]{};

        constexpr           Triangle2();
        explicit constexpr  Triangle2(const Float2 vertices[3])                             : m_vertices{ vertices[0], vertices[1], vertices[2] } {}
        constexpr           Triangle2(const Float2 v0, const Float2 v1, const Float2 v2)    : m_vertices{ v0, v1, v2 } {}
        explicit            Triangle2(const Vec2 vertices[3]);
                            Triangle2(const Vec2 v0, const Vec2 v1, const Vec2 v2);

        Float2&             operator[](const size_t index)          { NES_ASSERT(index < 3); return m_vertices[index]; }
        Float2              operator[](const size_t index) const    { NES_ASSERT(index < 3); return m_vertices[index]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the triangle. 
        //----------------------------------------------------------------------------------------------------
        Vec2                Centroid() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the area of the triangle. 
        //----------------------------------------------------------------------------------------------------
        float               Area() const                            { return math::Abs(SignedArea()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the area of the triangle. 
        //----------------------------------------------------------------------------------------------------
        float               SignedArea() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns whether the point is in or on the border of the Triangle. 
        //----------------------------------------------------------------------------------------------------
        bool                Contains(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
        //----------------------------------------------------------------------------------------------------
        Vec2                PointFromBaryCoordinates(float bary0, float bary1, float bary2) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculate the Barycentric coordinates for point p. 
        //----------------------------------------------------------------------------------------------------
        void                CalculateBarycentricCoordinate(const Vec2 p, float& bary0, float& bary1, float& bary2) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the closest point that is in or on the triangle from the query point.  
        //----------------------------------------------------------------------------------------------------
        Vec2                ClosestPointTo(const Vec2 queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the distance from the query point to the closest point on the triangle.
        //----------------------------------------------------------------------------------------------------
        float               Distance(const Vec2 queryPoint) const       { return std::sqrt(DistanceSqrTo(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the squared distance from the query point to the closest point on the triangle.
        //----------------------------------------------------------------------------------------------------
        float               DistanceSqrTo(const Vec2 queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return a triangle transformed by the matrix. 
        //----------------------------------------------------------------------------------------------------
        Triangle2           Transformed(const Mat44& m) const;
    };

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Triangle class that takes 3D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    struct Triangle
    {
        Float3              m_vertices[3]{};

        constexpr           Triangle();
        explicit constexpr  Triangle(const Float3 vertices[3])                          : m_vertices{ vertices[0], vertices[1], vertices[2] } {}
        constexpr           Triangle(const Float3 v0, const Float3 v1, const Float3 v2) : m_vertices{ v0, v1, v2 } {}
        explicit constexpr  Triangle(const Triangle2& triangle);
        explicit            Triangle(const Vec3 vertices[3]);
                            Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2);

        /// Index Operators
        Float3&             operator[](const size_t index)          { NES_ASSERT(index < 3); return m_vertices[index]; }
        Float3              operator[](const size_t index) const    { NES_ASSERT(index < 3); return m_vertices[index]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a vertex converted to a Vec3
        //----------------------------------------------------------------------------------------------------
        Vec3                GetVertex(const size_t index) const    { NES_ASSERT(index < 3); return m_vertices[index]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the center of the triangle. 
        //----------------------------------------------------------------------------------------------------
        Vec3                Centroid() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the area of the triangle. 
        //----------------------------------------------------------------------------------------------------
        float               Area() const                            { return math::Abs(SignedArea()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the signed area of the triangle. 
        //----------------------------------------------------------------------------------------------------
        float               SignedArea() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns whether the point is in or on the border of the Triangle. 
        //----------------------------------------------------------------------------------------------------
        bool                Contains(const Vec3& point) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
        //----------------------------------------------------------------------------------------------------
        Vec3                PointFromBaryCoordinates(float bary0, float bary1, float bary2) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculate the Barycentric coordinates for point p. 
        //----------------------------------------------------------------------------------------------------
        void                CalculateBarycentricCoordinate(const Vec3& p, float& bary0, float& bary1, float& bary2) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the closest point that is in or on the triangle from the query point.  
        //----------------------------------------------------------------------------------------------------
        Vec3                ClosestPointTo(const Vec3& queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the distance from the query point to the closest point on the triangle.
        //----------------------------------------------------------------------------------------------------
        float               Distance(const Vec3& queryPoint) const      { return std::sqrt(DistanceSqr(queryPoint)); }

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the squared distance from the query point to the closest point on the triangle.
        //----------------------------------------------------------------------------------------------------
        float               DistanceSqr(const Vec3& queryPoint) const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculates the plane normal of this Triangle. 
        //----------------------------------------------------------------------------------------------------
        Vec3                Normal() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Return a triangle transformed by the matrix. 
        //----------------------------------------------------------------------------------------------------
        Triangle            Transformed(const Mat44& m) const;
    };

    namespace geo
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the *signed* area of a 2D Triangle. Meaning, this will only use the XY components
        ///     of the a, b, c vertices.
        //----------------------------------------------------------------------------------------------------
        float CalculateSignedAreaOfTriangle(const Vec3 a, const Vec3 b, const Vec3 c);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculate the *signed* area of a 2D Triangle.
        //----------------------------------------------------------------------------------------------------
        float CalculateSignedAreaOfTriangle2D(const Vec2 a, const Vec2 b, const Vec2 c);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Test whether a point "p" lies in or on the triangle "abc".
        //----------------------------------------------------------------------------------------------------
        bool TriangleContainsPoint(const Vec3 a, const Vec3 b, const Vec3 c, const Vec3 p);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Test whether a point "p" lies in or on the triangle "abc".
        //----------------------------------------------------------------------------------------------------
        bool TriangleContainsPoint2D(const Vec2 a, const Vec2 b, const Vec2 c, const Vec2 p);
    }
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : The default constructor creates a Triangle with the vertices:
    ///     0 = (-0.5, -0.5);
    ///     1 = (0, 0.5);
    ///     2 = (0.5, -0.5);
    //----------------------------------------------------------------------------------------------------
    constexpr Triangle2::Triangle2()
    {
        m_vertices[0] = Float2(-0.5f, -0.5f);
        m_vertices[1] = Float2(0.f, 0.5f);
        m_vertices[2] = Float2(0.5f, -0.5f);
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : The default constructor creates a Triangle with the vertices:
    ///     0 = (-0.5, -0.5f, 0.f);
    ///     1 = (0, 0.5, 0.f);
    ///     2 = (0.5, -0.5);
    //----------------------------------------------------------------------------------------------------
    constexpr Triangle::Triangle()
    {
        m_vertices[0] = Float3(-0.5f, -0.5f, 0.f);
        m_vertices[1] = Float3(0.f, 0.5f, 0.f);
        m_vertices[2] = Float3(0.5f, -0.5f, 0.f);
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Explicit constructor for converting from a triangle expressed with 2D points to
    ///     a triangle with 3D points.
    //----------------------------------------------------------------------------------------------------
    constexpr Triangle::Triangle(const Triangle2& triangle)
    {
        m_vertices[0] = { triangle.m_vertices[0].x, triangle.m_vertices[0].y, 0.f };
        m_vertices[1] = { triangle.m_vertices[1].x, triangle.m_vertices[1].y, 0.f };
        m_vertices[2] = { triangle.m_vertices[2].x, triangle.m_vertices[2].y, 0.f };
    }
}
