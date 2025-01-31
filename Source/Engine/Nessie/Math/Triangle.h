// Triangles.h
#pragma once
#include "Ray.h"
#include "Vector3.h"

namespace nes
{
    namespace math
    {
        template <FloatingPointType Type>
        static constexpr TVector3<Type> CalculateBarycentricCoordinate(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c, const TVector3<Type>& p);

        template <FloatingPointType Type>
        static constexpr TVector3<Type> CalculateBarycentricCoordinate2D(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c, const TVector2<Type>& p);
        
        template <FloatingPointType Type>
        static constexpr float CalculateSignedAreaOfTriangle(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c);
        
        template <FloatingPointType Type>
        static constexpr float CalculateSignedAreaOfTriangle2D(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c);

        template <FloatingPointType Type>
        static constexpr bool TriangleContainsPoint(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c, const TVector3<Type>& p);

        template <FloatingPointType Type>
        static constexpr bool TriangleContainsPoint2D(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c, const TVector2<Type>& p);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Triangle class that takes 2D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TTriangle2
    {
        TVector2<Type> m_vertices[3]{};

        constexpr TTriangle2();
        constexpr TTriangle2(const TVector2<Type>& v0, const TVector2<Type>& v1, const TVector2<Type>& v2);
        explicit constexpr TTriangle2(const TVector2<Type> vertices[3]);

        TVector2<Type>& operator[](const size_t index);
        TVector2<Type> operator[](const size_t index) const;
        
        constexpr float Area() const;
        constexpr float SignedArea() const;
        constexpr bool ContainsPoint(const TVector2<Type>& point) const;
        constexpr TVector2<Type> GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const;
        void CalculateBarycentricCoordinate(const TVector2<Type>& p, Type& bary0, Type& bary1, Type& bary2) const;

        std::string ToString() const;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Triangle class that takes 3D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TTriangle3
    {
        TVector3<Type> m_vertices[3]{};

        constexpr TTriangle3();
        constexpr TTriangle3(const TVector3<Type>& v0, const TVector3<Type>& v1, const TVector3<Type>& v2);
        explicit constexpr TTriangle3(const TVector3<Type> vertices[3]);
        explicit constexpr TTriangle3(const TTriangle2<Type>& triangle);

        TVector3<Type>& operator[](const size_t index);
        TVector3<Type> operator[](const size_t index) const;

        constexpr float Area() const;
        constexpr float SignedArea() const;
        constexpr bool ContainsPoint(const TVector3<Type>& point) const;
        constexpr TVector3<Type> GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const;
        void CalculateBarycentricCoordinate(const TVector3<Type>& p, Type& bary0, Type& bary1, Type& bary2) const;
        
        std::string ToString() const;
    };

    using Triangle2f = TTriangle2<float>;
    using Triangle2d = TTriangle2<double>;
    using Triangle2D = TTriangle2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Triangle3f = TTriangle3<float>;
    using Triangle3d = TTriangle3<double>;
    using Triangle = TRay3<NES_MATH_DEFAULT_REAL_TYPE>;
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
        constexpr TVector3<Type> CalculateBarycentricCoordinate(const TVector3<Type>& a, const TVector3<Type>& b,
            const TVector3<Type>& c, const TVector3<Type>& p)
        {
#if 1
            // Implementation derived from first defining a 2x2 system of linear equations, then
            // using Cramer's Rule and the Dot Product. Honestly a bit confusing to read through...
            // Barycentric Coordinates are on pg 47-48 of "Real-Time Collision Detection".
            const TVector3<Type> v0 = b - a;
            const TVector3<Type> v1 = c - a;
            const TVector3<Type> v2 = p - a;

            const Type dot00 = TVector3<Type>::Dot(v0, v0);
            const Type dot01 = TVector3<Type>::Dot(v0, v1);
            const Type dot11 = TVector3<Type>::Dot(v1, v1);
            const Type dot20 = TVector3<Type>::Dot(v2, v0);
            const Type dot21 = TVector3<Type>::Dot(v2, v1);
            
            const Type denominator = dot00 * dot11 - dot01 * dot01;

            TVector3<Type> result{};
            result.y = (dot11 * dot20 - dot01 * dot21) / denominator;
            result.z = (dot00 * dot21 - dot01 * dot20) / denominator;
            result.x = static_cast<Type>(1.0) - result.y - result.z;
            return result;
#else
            // Another implementation is on pg 50-51 of "Real-Time Collision Detection".
            
            // Unnormailzed triangle normal
            const Vector3<Type> normal = Vector3<Type>::Cross(b - a, c - a);

            // Nominators and one-over-denominator for u and v ratios (x & y of the result vector).
            float numeratorU, numeratorV, oneOverDenom;

            // Largest absolute component to determine the projection plane.
            float x = math::Abs(normal.x);
            float y = math::Abs(normal.y);
            float z = math::Abs(normal.z);

            // Compute the area on the plane of largest projection.
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
        constexpr TVector3<Type> CalculateBarycentricCoordinate2D(const TVector2<Type>& a, const TVector2<Type>& b,
            const TVector2<Type>& c, const TVector2<Type>& p)
        {
            return CalculateBarycentricCoordinate(TVector3<Type>(a), TVector3<Type>(b), TVector3<Type>(c), TVector3<Type>(p));
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Calculate the *signed* area of a 2D Triangle. Meaning, this will only use the XY components
        ///             of the a, b, c vertices.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr float CalculateSignedAreaOfTriangle(const TVector3<Type>& a, const TVector3<Type>& b,
            const TVector3<Type>& c)
        {
            return (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);
        }

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Calculate the *signed* area of a 2D Triangle.
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr float CalculateSignedAreaOfTriangle2D(const TVector2<Type>& a, const TVector2<Type>& b,
            const TVector2<Type>& c)
        {
            return (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      This uses Barycentric Coordinates to determine if the point is contained by the triangle. This is true
        //      if the u, v, w coordinates are all within the range [0, 1].
        //		
        ///		@brief : Test whether a point "p" lies in or on the triangle "abc".
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr bool TriangleContainsPoint(const TVector3<Type>& a, const TVector3<Type>& b, const TVector3<Type>& c,
            const TVector3<Type>& p)
        {
            const TVector3<Type> baryCoord = CalculateBarycentricCoordinate(a, b, c, p);
            return baryCoord.y >= 0.f && baryCoord.y >= 0.f && baryCoord.z + baryCoord.x <= 1.f;
        }

        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      This uses Barycentric Coordinates to determine if the point is contained by the triangle. This is true
        //      if the u, v, w coordinates are all within the range [0, 1].
        //		
        ///		@brief : Test whether a point "p" lies in or on the triangle "abc".
        //----------------------------------------------------------------------------------------------------
        template <FloatingPointType Type>
        constexpr bool TriangleContainsPoint2D(const TVector2<Type>& a, const TVector2<Type>& b, const TVector2<Type>& c,
            const TVector2<Type>& p)
        {
            const TVector3<Type> baryCoord = CalculateBarycentricCoordinate2D(a, b, c, p);
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
    constexpr TTriangle2<Type>::TTriangle2()
    {
        m_vertices[0] = TVector2<Type>(-0.5f, 0.f);
        m_vertices[1] = TVector2<Type>(0.f, 1.f);
        m_vertices[2] = TVector2<Type>(0.5f, 0.f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in the 3 Vertices for the triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TTriangle2<Type>::TTriangle2(const TVector2<Type>& v0, const TVector2<Type>& v1, const TVector2<Type>& v2)
    {
        m_vertices[0] = v0;
        m_vertices[1] = v1;
        m_vertices[2] = v2;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in an array of Vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TTriangle2<Type>::TTriangle2(const TVector2<Type> vertices[3])
    {
        m_vertices[0] = vertices[0];
        m_vertices[1] = vertices[1];
        m_vertices[2] = vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float TTriangle2<Type>::Area() const
    {
        return math::Abs(SignedArea());
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the *signed* Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float TTriangle2<Type>::SignedArea() const
    {
        return math::CalculateSignedAreaOfTriangle2D(m_vertices[0], m_vertices[1], m_vertices[2]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the point is in or on the border of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TTriangle2<Type>::ContainsPoint(const TVector2<Type>& point) const
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
    constexpr TTriangle3<Type>::TTriangle3()
    {
        m_vertices[0] = TVector3<Type>(-0.5f, 0.f, 0.f);
        m_vertices[1] = TVector3<Type>(0.f, 1.f, 0.f);
        m_vertices[2] = TVector3<Type>(0.5f, 0.f, 0.f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in the 3 Vertices for the triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TTriangle3<Type>::TTriangle3(const TVector3<Type>& v0, const TVector3<Type>& v1, const TVector3<Type>& v2)
    {
        m_vertices[0] = v0;
        m_vertices[1] = v1;
        m_vertices[2] = v2;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in an array of Vertices. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TTriangle3<Type>::TTriangle3(const TVector3<Type> vertices[3])
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
    constexpr TTriangle3<Type>::TTriangle3(const TTriangle2<Type>& triangle)
    {
        m_vertices[0] = triangle.m_vertices[0];
        m_vertices[1] = triangle.m_vertices[1];
        m_vertices[2] = triangle.m_vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float TTriangle3<Type>::Area() const
    {
        return math::Abs(SignedArea());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the *signed* Area of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr float TTriangle3<Type>::SignedArea() const
    {
        return math::CalculateSignedAreaOfTriangle2D(m_vertices[0], m_vertices[1], m_vertices[2]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the point is in or on the border of the Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr bool TTriangle3<Type>::ContainsPoint(const TVector3<Type>& point) const
    {
        return math::TriangleContainsPoint(m_vertices[0], m_vertices[1], m_vertices[2], point);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TTriangle2<Type>::GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    template <FloatingPointType Type>
    TVector2<Type>& TTriangle2<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    template <FloatingPointType Type>
    TVector2<Type> TTriangle2<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinate for the point p.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TTriangle2<Type>::CalculateBarycentricCoordinate(const TVector2<Type>& p, Type& bary0, Type& bary1,
        Type& bary2) const
    {
        const TVector3<Type> baryCoordinates = math::CalculateBarycentricCoordinate(m_vertices[0], m_vertices[1], m_vertices[2], p);
        bary0 = baryCoordinates.x;
        bary1 = baryCoordinates.y;
        bary2 = baryCoordinates.z;
    }

    template <FloatingPointType Type>
    std::string TTriangle2<Type>::ToString() const
    {
        return CombineIntoString("A: ", m_vertices[0], "\nB: ", m_vertices[1], ",\nC: ", m_vertices[2]);
    }

    template <FloatingPointType Type>
    TVector3<Type>& TTriangle3<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    template <FloatingPointType Type>
    TVector3<Type> TTriangle3<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 3);
        return m_vertices[index];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TTriangle3<Type>::GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinates for point p. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TTriangle3<Type>::CalculateBarycentricCoordinate(const TVector3<Type>& p, Type& bary0, Type& bary1, Type& bary2) const
    {
        const TVector3<Type> baryCoordinates = math::CalculateBarycentricCoordinate(m_vertices[0], m_vertices[1], m_vertices[2], p);
        bary0 = baryCoordinates.x;
        bary1 = baryCoordinates.y;
        bary2 = baryCoordinates.z;
    }

    template <FloatingPointType Type>
    std::string TTriangle3<Type>::ToString() const
    {
        return CombineIntoString("A: ", m_vertices[0], "\nB: ", m_vertices[1], "\nC: ", m_vertices[2]);
    }

}
