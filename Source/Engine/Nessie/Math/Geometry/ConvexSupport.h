// ConvexSupport.h
#pragma once
#include "Math/Matrix.h"

namespace nes
{
    template <typename Type>
    concept ValidConvexObjectType = requires(Type object, Vector3 direction)
    {
        { object.GetSupport(direction) } -> std::same_as<Vector3>;  
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper struct to get the support point for a convex object.
    ///     This structure transforms a convex object (supports on uniform scaling). 
    //----------------------------------------------------------------------------------------------------
    template <typename ConvexObject>
    struct TransformedConvexObject
    {
        Mat4                m_transform;
        const ConvexObject& m_object;
        
        TransformedConvexObject(const Mat4& transform, const ConvexObject& convexObject)
            : m_transform(transform)
            , m_object(convexObject)
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape.
        //----------------------------------------------------------------------------------------------------
        Vector3 GetSupport(const Vector3& direction) const
        {
            return m_transform.TransformPoint(m_object.GetSupport(m_transform.TransformVectorTranspose(direction)));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vertices of the fae that faces 'direction' the most.
        //----------------------------------------------------------------------------------------------------
        template <class VertexArray>
        void    GetSupportingFace(const Vector3& direction, VertexArray& outVertices) const
        {
            m_object.GetSupportingFace(m_transform.TransformVectorTranspose(direction), outVertices);

            for (Vector3& vertex : outVertices)
                vertex = m_transform.TransformPoint(vertex);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper struct that adds a convex radius when calling GetSupport() on a ConvexShape. 
    //----------------------------------------------------------------------------------------------------
    template <typename ConvexObject>
    struct AddConvexRadius
    {
        const ConvexObject& m_object;
        float               m_radius;
        
        AddConvexRadius(const ConvexObject& convexObject, const float radius)
            : m_object(convexObject)
            , m_radius(radius)
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape.
        //----------------------------------------------------------------------------------------------------
        Vector3 GetSupport(const Vector3& direction) const
        {
            float length = direction.Magnitude();
            return length > 0.f ? m_object.GetSupport(direction) + (m_radius / length) * direction : m_object.GetSupport(direction);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper struct to perform a Minkowski difference A - B.
    //----------------------------------------------------------------------------------------------------
    template <typename ConvexObjectA, typename ConvexObjectB>
    struct MinkowskiDifference
    {
        const ConvexObjectA& m_objectA;
        const ConvexObjectA& m_objectB;

        MinkowskiDifference(const ConvexObjectA& objectA, const ConvexObjectA& objectB)
            : m_objectA(objectA)
            , m_objectB(objectB)
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape. 
        //----------------------------------------------------------------------------------------------------
        Vector3 GetSupport(const Vector3& direction) const
        {
            return m_objectA.GetSupport(direction) - m_objectB.GetSupport(-direction);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct that wraps a point so that it can be used with convex collision detection. 
    //----------------------------------------------------------------------------------------------------
    struct PointConvexSupport
    {
        Vector3 m_point;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape. 
        //----------------------------------------------------------------------------------------------------
        Vector3 GetSupport([[maybe_unused]] const Vector3& direction) const
        {
            return m_point;
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct that wraps a triangle so that it can be used with convex collision detection. 
    //----------------------------------------------------------------------------------------------------
    struct TriangleConvexSupport
    {
        /// The three vertices of the triangle.
        Vector3 m_vert1;
        Vector3 m_vert2;
        Vector3 m_vert3;

        TriangleConvexSupport(const Vector3& vert1, const Vector3& vert2, const Vector3& vert3)
            : m_vert1(vert1)
            , m_vert2(vert2)
            , m_vert3(vert3)
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape. 
        //----------------------------------------------------------------------------------------------------
        Vector3 GetSupport(const Vector3& direction) const
        {
            // Project vertices onto the direction
            const float d1 = m_vert1.Dot(direction);
            const float d2 = m_vert2.Dot(direction);
            const float d3 = m_vert3.Dot(direction);

            // Return vertex with the largest projection
            if (d1 > d2)
            {
                if (d1 > d3)
                    return m_vert1;
                else
                    return m_vert3;
            }
            else
            {
                if (d2 > d3)
                    return m_vert2;
                else
                    return m_vert3;
            }
        }

        template <class VertexArray>
        void    GetSupportingFace([[maybe_unused]] const Vector3& direction, VertexArray& outVertices)
        {
            outVertices.push_back(m_vert1);
            outVertices.push_back(m_vert2);
            outVertices.push_back(m_vert3);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct that wraps a polygon so that it can be used with convex collision detection. 
    //----------------------------------------------------------------------------------------------------
    template <typename VertexArray>
    struct PolygonConvexSupport
    {
        const VertexArray& m_vertices;
        
        explicit    PolygonConvexSupport(const VertexArray& vertices) : m_vertices(vertices) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the support vector for this convex shape. 
        //----------------------------------------------------------------------------------------------------
        Vector3     GetSupport(const Vector3& direction) const
        {
            Vector3 supportPoint = m_vertices[0];
            float bestDot = m_vertices[0].Dot(direction);

            for (typename VertexArray::const_iterator it = m_vertices.begin(); it != m_vertices.end(); ++it)
            {
                float dot = it->Dot(direction);
                if (dot > bestDot)
                {
                    bestDot = dot;
                    supportPoint = *it;
                }
            }

            return supportPoint;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vertices of the fae that faces 'direction' the most.
        //----------------------------------------------------------------------------------------------------
        template <typename VertexArrayArg>
        void        GetSupportingFace([[maybe_unused]] const Vector3& direction, VertexArrayArg& outVertices)
        {
            for (Vector3& vertex : m_vertices)
                outVertices.push_back(vertex);
        }
    };
}