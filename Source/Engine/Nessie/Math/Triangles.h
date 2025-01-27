// Triangles.h
#pragma once
#include <StructuredQueryCondition.h>

#include "Vector3.h"

namespace nes
{
    namespace math
    {
        template <FloatingPointType Type>
        static constexpr Vector3<Type> CalculateBarycentricCoordinate(const Vector3<Type>& a, const Vector3<Type>& b, const Vector3<Type>& c, const Vector3<Type>& p);

        template <FloatingPointType Type>
        static constexpr Vector3<Type> CalculateBarycentricCoordinate2D(const Vector2<Type>& a, const Vector2<Type>& b, const Vector2<Type>& c, const Vector2<Type>& p);
        
        template <FloatingPointType Type>
        static constexpr float CalculateSignedAreaOfTriangle(const Vector3<Type>& a, const Vector3<Type>& b, const Vector3<Type>& c);
        
        template <FloatingPointType Type>
        static constexpr float CalculateSignedAreaOfTriangle2D(const Vector2<Type>& a, const Vector2<Type>& b, const Vector2<Type>& c);

        template <FloatingPointType Type>
        static constexpr bool TriangleContainsPoint(const Vector3<Type>& a, const Vector3<Type>& b, const Vector3<Type>& c, const Vector3<Type>& p);

        template <FloatingPointType Type>
        static constexpr bool TriangleContainsPoint2D(const Vector2<Type>& a, const Vector2<Type>& b, const Vector2<Type>& c, const Vector2<Type>& p);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Triangle class that takes 2D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct Triangle2D
    {
        Vector2<Type> m_vertices[3]{};

        constexpr Triangle2D();
        constexpr Triangle2D(const Vector2<Type>& v0, const Vector2<Type>& v1, const Vector2<Type>& v2);
        explicit constexpr Triangle2D(const Vector2<Type> vertices[3]);

        Vector2<Type>& operator[](const size_t index);
        Vector2<Type> operator[](const size_t index) const;
        
        constexpr float Area() const;
        constexpr float SignedArea() const;
        constexpr bool ContainsPoint(const Vector2<Type>& point) const;
        constexpr Vector2<Type> GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const;
        void CalculateBarycentricCoordinate(const Vector2<Type>& p, Type& bary0, Type& bary1, Type& bary2) const;

        std::string ToString() const;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Triangle class that takes 3D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct Triangle
    {
        Vector3<Type> m_vertices[3]{};

        constexpr Triangle();
        constexpr Triangle(const Vector3<Type>& v0, const Vector3<Type>& v1, const Vector3<Type>& v2);
        explicit constexpr Triangle(const Vector3<Type> vertices[3]);
        explicit constexpr Triangle(const Triangle2D<Type>& triangle);

        Vector3<Type>& operator[](const size_t index);
        Vector3<Type> operator[](const size_t index) const;

        constexpr float Area() const;
        constexpr float SignedArea() const;
        constexpr bool ContainsPoint(const Vector3<Type>& point) const;
        constexpr Vector3<Type> GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const;
        void CalculateBarycentricCoordinate(const Vector3<Type>& p, Type& bary0, Type& bary1, Type& bary2) const;
        
        std::string ToString() const;
    };
}

namespace nes
{
    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        ///		@brief : Calculate the Barycentric Coordinates for point P, using the triangle made up of the points
        ///              a, b, c. The resulting vector's will contain the scalar values to multiply each of a, b, and c
        ///              by to get the point P. 
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr Vector3<Type> CalculateBarycentricCoordinate(const Vector3<Type>& a, const Vector3<Type>& b,
            const Vector3<Type>& c, const Vector3<Type>& p)
        {
#if 1
            // Implementation derived from first defining a 2x2 system of linear equations, then
            // using Cramer's Rule and the Dot Product. Honestly a bit confusing to read through...
            // Barycentric Coordinates are on pg 47-48 of "Real-Time Collision Detection".
            const Vector3<Type> v0 = b - a;
            const Vector3<Type> v1 = c - a;
            const Vector3<Type> v2 = p - a;

            const Type dot00 = Vector3<Type>::Dot(v0, v0);
            const Type dot01 = Vector3<Type>::Dot(v0, v1);
            const Type dot11 = Vector3<Type>::Dot(v1, v1);
            const Type dot20 = Vector3<Type>::Dot(v2, v0);
            const Type dot21 = Vector3<Type>::Dot(v2, v1);

            const Type denominator = dot00 * dot11 - dot01 * dot01;

            Vector3<Type> result{};
            result.y = (dot11 * dot20 - dot01 * dot21) / denominator;
            result.z = (dot00 * dot21 - dot01 * dot20) / denominator;
            result.x = static_cast<Type>(1.0) - result.y - result.z;
            return result;
#else
            // Barycentric Coordinates are on pg 50-51 of "Real-Time Collision Detection".
            
            // Unnormailzed triangle normal
            const Vector3<Type> normal = Vector3<Type>::Cross(b - a, c - a);

            // Nominators and one-over-denominator for u and v ratios (x & y of the result vector).
            float numeratorU, numeratorV, oneOverDenom;

            // Largest absolute component to determine the projection plane.
            float x = math::Abs(normal.x);
            float y = math::Abs(normal.y);
            float z = math::Abs(normal.z);

            // Compute the ares in plane of largest projection.
            if (x >= y && x >= z)
            {
                // x is largest, project onto the yz plane
                numeratorU = CalculateSignedAreaOfTriangle2D(Vector2<Type>(p.y, p.z), Vector2<Type>(b.y, b.z), Vector2<Type>(c.y, c.z)); // Area of PBC in yz plane
                numeratorV = CalculateSignedAreaOfTriangle2D(Vector2<Type>(p.y, p.z), Vector2<Type>(c.y, c.z), Vector2<Type>(a.y, a.z)); // Area of PCA in yz plane
                oneOverDenom = 1.0f / normal.x;                                                                                          // 1 /(2*area of ABC in the yz plane).
            }

            else if (y >= x && y >= z)
            {
                // y is largest, project onto the xz plane
                numeratorU = CalculateSignedAreaOfTriangle2D(Vector2<Type>(p.x, p.z), Vector2<Type>(b.x, b.z), Vector2<Type>(c.x, c.z));
                numeratorV = CalculateSignedAreaOfTriangle2D(Vector2<Type>(p.x, p.z), Vector2<Type>(c.x, c.z), Vector2<Type>(a.x, a.z));
                oneOverDenom = 1.0f / -normal.y;
            }

            else
            {
                // z is largest, project onto the xy plane
                numeratorU = CalculateSignedAreaOfTriangle2D(Vector2<Type>(p.x, p.y), Vector2<Type>(b.x, b.y), Vector2<Type>(c.x, c.y));
                numeratorV = CalculateSignedAreaOfTriangle2D(Vector2<Type>(p.x, p.y), Vector2<Type>(c.x, c.y), Vector2<Type>(a.x, a.y));
                oneOverDenom = 1.0f / normal.z;    
            }

            Vector3<Type> result{};
            result.x = numeratorU * oneOverDenom; // u
            result.y = numeratorV * oneOverDenom; // v
            result.z = 1.f - result.x - result.y; // w = 1 - u - v
            return result;
#endif
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Calculate the Barycentric Coordinates for point P, using the triangle made up of the points
        ///              a, b, c. The resulting vector's will contain the scalar values to multiply each of a, b, and c
        ///              by to get the point P. 
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr Vector3<Type> CalculateBarycentricCoordinate2D(const Vector2<Type>& a, const Vector2<Type>& b,
            const Vector2<Type>& c, const Vector2<Type>& p)
        {
            return CalculateBarycentricCoordinate(Vector3<Type>(a), Vector3<Type>(b), Vector3<Type>(c), Vector3<Type>(p));
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Calculate the *signed* area of a 2D Triangle. Meaning, this will only use the XY components
        ///             of the a, b, c vertices.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr float CalculateSignedAreaOfTriangle(const Vector3<Type>& a, const Vector3<Type>& b,
            const Vector3<Type>& c)
        {
            return (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Calculate the *signed* area of a 2D Triangle.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr float CalculateSignedAreaOfTriangle2D(const Vector2<Type>& a, const Vector2<Type>& b,
            const Vector2<Type>& c)
        {
            return (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      This uses Barycentric Coordinates to determine if the point is contained by the triangle. This is true
        //      if the u, v, w coordinates are all 
        //		
        ///		@brief : Test whether a point "p" lies in or on the triangle "abc".
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr bool TriangleContainsPoint(const Vector3<Type>& a, const Vector3<Type>& b, const Vector3<Type>& c,
            const Vector3<Type>& p)
        {
            const Vector3<Type> baryCoord = CalculateBarycentricCoordinate(a, b, c, p);
            return baryCoord.y >= 0.f && baryCoord.y >= 0.f && baryCoord.z + baryCoord.x <= 1.f;
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      This uses Barycentric Coordinates to determine if the point is contained by the triangle. This is true
        //      if the u, v, w coordinates are all 
        //		
        ///		@brief : Test whether a point "p" lies in or on the triangle "abc".
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr bool TriangleContainsPoint2D(const Vector2<Type>& a, const Vector2<Type>& b, const Vector2<Type>& c,
            const Vector2<Type>& p)
        {
            const Vector3<Type> baryCoord = CalculateBarycentricCoordinate2D(a, b, c, p);
            return baryCoord.y >= 0.f && baryCoord.z >= 0.f && (baryCoord.y + baryCoord.z) <= 1.f;
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Default constructor creates a Triangle with the vertices:
    ///              0 = (-0.5, 0);
    ///              1 = (0, 1);
    ///              2 = (0.5, 0);
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle2D<Type>::Triangle2D()
    {
        m_vertices[0] = Vector2<Type>(-0.5f, 0.f);
        m_vertices[1] = Vector2<Type>(0.f, 1.f);
        m_vertices[2] = Vector2<Type>(0.5f, 0.f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in the 3 Vertices for the triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle2D<Type>::Triangle2D(const Vector2<Type>& v0, const Vector2<Type>& v1, const Vector2<Type>& v2)
    {
        m_vertices[0] = v0;
        m_vertices[1] = v1;
        m_vertices[2] = v2;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in an array of Vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle2D<Type>::Triangle2D(const Vector2<Type> vertices[3])
    {
        m_vertices[0] = vertices[0];
        m_vertices[1] = vertices[1];
        m_vertices[2] = vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float Triangle2D<Type>::Area() const
    {
        return math::Abs(SignedArea());
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the *signed* Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float Triangle2D<Type>::SignedArea() const
    {
        return math::CalculateSignedAreaOfTriangle2D(m_vertices[0], m_vertices[1], m_vertices[2]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the point is in or on the border of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool Triangle2D<Type>::ContainsPoint(const Vector2<Type>& point) const
    {
        return math::TriangleContainsPoint2D(m_vertices[0], m_vertices[1], m_vertices[2], point);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Default constructor creates a Triangle with the vertices:
    ///              0 = (-0.5, 0, 0);
    ///              1 = (0, 1, 0);
    ///              2 = (0.5, 0, 0); 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle<Type>::Triangle()
    {
        m_vertices[0] = Vector3<Type>(-0.5f, 0.f, 0.f);
        m_vertices[1] = Vector3<Type>(0.f, 1.f, 0.f);
        m_vertices[2] = Vector3<Type>(0.5f, 0.f, 0.f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in the 3 Vertices for the triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle<Type>::Triangle(const Vector3<Type>& v0, const Vector3<Type>& v1, const Vector3<Type>& v2)
    {
        m_vertices[0] = v0;
        m_vertices[1] = v1;
        m_vertices[2] = v2;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in an array of Vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle<Type>::Triangle(const Vector3<Type> vertices[3])
    {
        m_vertices[0] = vertices[0];
        m_vertices[1] = vertices[1];
        m_vertices[2] = vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Explicit constructor for converting from a triangle expressed with 2D points to
    ///             a triangle with 3D points.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Triangle<Type>::Triangle(const Triangle2D<Type>& triangle)
    {
        m_vertices[0] = triangle.m_vertices[0];
        m_vertices[1] = triangle.m_vertices[1];
        m_vertices[2] = triangle.m_vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float Triangle<Type>::Area() const
    {
        return math::Abs(SignedArea());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the *signed* Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float Triangle<Type>::SignedArea() const
    {
        return math::CalculateSignedAreaOfTriangle2D(m_vertices[0], m_vertices[1], m_vertices[2]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the point is in or on the border of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool Triangle<Type>::ContainsPoint(const Vector3<Type>& point) const
    {
        return math::TriangleContainsPoint(m_vertices[0], m_vertices[1], m_vertices[2], point);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Vector2<Type> Triangle2D<Type>::GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    template <FloatingPointType Type>
    Vector2<Type>& Triangle2D<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    template <FloatingPointType Type>
    Vector2<Type> Triangle2D<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinate for the point p.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void Triangle2D<Type>::CalculateBarycentricCoordinate(const Vector2<Type>& p, Type& bary0, Type& bary1,
        Type& bary2) const
    {
        const Vector3<Type> baryCoordinates = math::CalculateBarycentricCoordinate(m_vertices[0], m_vertices[1], m_vertices[2], p);
        bary0 = baryCoordinates.x;
        bary1 = baryCoordinates.y;
        bary2 = baryCoordinates.z;
    }

    template <FloatingPointType Type>
    std::string Triangle2D<Type>::ToString() const
    {
        return CombineIntoString("A: ", m_vertices[0], "\nB: ", m_vertices[1], ",\nC: ", m_vertices[2]);
    }

    template <FloatingPointType Type>
    Vector3<Type>& Triangle<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    template <FloatingPointType Type>
    Vector3<Type> Triangle<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Vector3<Type> Triangle<Type>::GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinates for point p. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void Triangle<Type>::CalculateBarycentricCoordinate(const Vector3<Type>& p, Type& bary0, Type& bary1, Type& bary2) const
    {
        const Vector3<Type> baryCoordinates = math::CalculateBarycentricCoordinate(m_vertices[0], m_vertices[1], m_vertices[2], p);
        bary0 = baryCoordinates.x;
        bary1 = baryCoordinates.y;
        bary2 = baryCoordinates.z;
    }

    template <FloatingPointType Type>
    std::string Triangle<Type>::ToString() const
    {
        return CombineIntoString("A: ", m_vertices[0], "\nB: ", m_vertices[1], "\nC: ", m_vertices[2]);
    }

}
