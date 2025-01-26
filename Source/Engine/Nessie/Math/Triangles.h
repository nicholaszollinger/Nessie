// Triangles.h
#pragma once
#include "Vector3.h"

namespace nes
{
    namespace math
    {
        template <ScalarType Type>
        static constexpr Vector3<Type> CalculateBarycentricCoordinate(const Vector3<Type>& a, const Vector3<Type>& b, const Vector3<Type>& c, const Vector3<Type>& p);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Triangle class that takes 2D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct Triangle2D
    {
        Vector2<Type> m_vertices[3]{};

        constexpr Triangle2D() = default;
        constexpr Triangle2D(const Vector2<Type>& v0, const Vector2<Type>& v1, const Vector2<Type>& v2);
        explicit constexpr Triangle2D(const Vector2<Type> vertices[3]);
        
        constexpr Vector2<Type> GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const;
        void CalculateBarycentricCoordinate(const Vector2<Type>& p, Type& bary0, Type& bary1, Type& bary2) const;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Triangle class that takes 3D points as vertices. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct Triangle
    {
        Vector3<Type> m_vertices[3]{};

        constexpr Triangle() = default;
        constexpr Triangle(const Vector3<Type>& v0, const Vector3<Type>& v1, const Vector3<Type>& v2);
        explicit constexpr Triangle(const Vector3<Type> vertices[3]);

        constexpr Vector3<Type> GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const;
        void CalculateBarycentricCoordinate(const Vector3<Type>& p, Type& bary0, Type& bary1, Type& bary2) const;
    };
}

namespace nes
{
    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      Barycentric Coordinates are on pg 47-48 of "Real-Time Collision Detection".
        //		
        ///		@brief : Calculate the Barycentric Coordinates for point P, using the triangle made up of the points
        ///              a, b, c. The resulting vector's will contain the scalar values to multiply each of a, b, and c
        ///              by to get the point P. 
        //----------------------------------------------------------------------------------------------------
        template <ScalarType Type>
        constexpr Vector3<Type> CalculateBarycentricCoordinate(const Vector3<Type>& a, const Vector3<Type>& b,
            const Vector3<Type>& c, const Vector3<Type>& p)
        {
            const Vector3<Type> v0 = b - a;
            const Vector3<Type> v1 = c - a;
            const Vector3<Type> v2 = p - a;

            const Type dot00 = Vector3<Type>::Dot(v0, v0);
            const Type dot01 = Vector3<Type>::Dot(v0, v1);
            const Type dot11 = Vector3<Type>::Dot(v1, v1);
            const Type dot20 = Vector3<Type>::Dot(v2, v0);
            const Type dot21 = Vector3<Type>::Dot(v2, v1);

            const Type denom = dot00 * dot11 - dot01 * dot01;

            Vector3<Type> result{};
            result.y = (dot11 * dot20 - dot01 * dot21) / denom;
            result.z = (dot00 * dot21 - dot01 * dot20) / denom;
            result.x = 1.0 - result.y - result.z;
            return result;
        }
    }
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in the 3 Vertices for the triangle. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Triangle2D<Type>::Triangle2D(const Vector2<Type>& v0, const Vector2<Type>& v1, const Vector2<Type>& v2)
    {
        m_vertices[0] = v0;
        m_vertices[1] = v1;
        m_vertices[2] = v2;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in an array of Vertices. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Triangle2D<Type>::Triangle2D(const Vector2<Type> vertices[3])
    {
        m_vertices[0] = vertices[0];
        m_vertices[1] = vertices[1];
        m_vertices[2] = vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in the 3 Vertices for the triangle. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Triangle<Type>::Triangle(const Vector3<Type>& v0, const Vector3<Type>& v1, const Vector3<Type>& v2)
    {
        m_vertices[0] = v0;
        m_vertices[1] = v1;
        m_vertices[2] = v2;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructor that takes in an array of Vertices. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Triangle<Type>::Triangle(const Vector3<Type> vertices[3])
    {
        m_vertices[0] = vertices[0];
        m_vertices[1] = vertices[1];
        m_vertices[2] = vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Vector2<Type> Triangle2D<Type>::GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinate for the point p.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    void Triangle2D<Type>::CalculateBarycentricCoordinate(const Vector2<Type>& p, Type& bary0, Type& bary1,
        Type& bary2) const
    {
        const Vector3<Type> baryCoordinates = math::CalculateBarycentricCoordinate(m_vertices[0], m_vertices[1], m_vertices[2], p);
        bary0 = baryCoordinates.x;
        bary1 = baryCoordinates.y;
        bary2 = baryCoordinates.z;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the point in or on the triangle that corresponds to the given barycentric coordinates.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Vector3<Type> Triangle<Type>::GetBarycentricPoint(Type bary0, Type bary1, Type bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinates for point p. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    void Triangle<Type>::CalculateBarycentricCoordinate(const Vector3<Type>& p, Type& bary0, Type& bary1, Type& bary2) const
    {
        const Vector3<Type> baryCoordinates = math::CalculateBarycentricCoordinate(m_vertices[0], m_vertices[1], m_vertices[2], p);
        bary0 = baryCoordinates.x;
        bary1 = baryCoordinates.y;
        bary2 = baryCoordinates.z;
    }
}
