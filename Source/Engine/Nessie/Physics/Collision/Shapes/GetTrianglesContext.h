// GetTrianglesContext.h
#pragma once
#include "Physics/Collision/Shapes/Shape.h"

// [TODO]: Physics Materials.

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation of GetTrianglesStart/Next that uses a fixed list of vertices for the triangles.
    ///     These are transformed into world space when getting the triangles.
    //----------------------------------------------------------------------------------------------------
    class GetTrianglesContextVertexList
    {
        Mat4            m_localToWorld;
        const Vector3*  m_pTriangleVertices;
        size_t          m_numTriangleVertices;
        size_t          m_currentVertex;
        // Physics Material
        bool            m_isInsideOut;
        
    public:
        GetTrianglesContextVertexList(const Vector3& positionCOM, const Quat& rotation, const Vector3& scale, const Mat4& localTransform, const Vector3* triangleVertices, size_t numTriangleVertices);

        //----------------------------------------------------------------------------------------------------
        /// @see Shape::GetTrianglesNext()  
        //----------------------------------------------------------------------------------------------------
        int GetTrianglesNext(int maxTrianglesRequested, Float3* pOutTriangleVertices);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function that creates a vertex list of a half unit sphere (top part).
        //----------------------------------------------------------------------------------------------------
        template <typename VertexArray>
        static void CreateHalfUnitSphereTop(VertexArray& vertices, const int detailLevel);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function that creates a vertex list of a half unit sphere (bottom part).
        //----------------------------------------------------------------------------------------------------
        template <typename VertexArray>
        static void CreateHalfUnitSphereBottom(VertexArray& vertices, const int detailLevel);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function that creates an open cylinder of half height 1 and radius 1. 
        //----------------------------------------------------------------------------------------------------
        template <typename VertexArray>
        static void CreateUnitOpenCylinder(VertexArray& vertices, const int detailLevel);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function for creating a sphere. 
        //----------------------------------------------------------------------------------------------------
        template <typename VertexArray>
        static void CreateUnitSphereHelper(VertexArray& vertices, const Vector3& inV1, const Vector3& inV2, const Vector3& inV3, const int detailLevel);
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation of GetTrianglesStart/Next that uses a multiple fixed lists of vertices for
    ///     the triangles. These are transformed into world space when getting the triangles. 
    //----------------------------------------------------------------------------------------------------
    class GetTrianglesContextMultiVertexList
    {
        struct Part
        {
            Mat4            m_localToWorld;
            const Vector3*  m_pTriangleVertices;
            size_t          m_numTriangleVertices;
        };
        
        // [TODO]: 
        // Supposed to be a StaticArray
        std::vector<Part>   m_parts;
        unsigned            m_currentPart;
        size_t              m_currentVertex;
        // Physics Material
        bool                m_isInsideOut;
        
    public:
        GetTrianglesContextMultiVertexList(bool isInsideOut);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a mesh part and its transform. 
        //----------------------------------------------------------------------------------------------------
        void AddPart(const Mat4& localToWorld, const Vector3* pTriangleVertices, size_t numTriangleVertices);

        //----------------------------------------------------------------------------------------------------
        /// @see Shape::GetTrianglesNext()  
        //----------------------------------------------------------------------------------------------------
        int GetTrianglesNext(int maxTrianglesRequested, Float3* pOutTriangleVertices);
    };

    template <typename VertexArray>
    void GetTrianglesContextVertexList::CreateHalfUnitSphereTop(VertexArray& vertices, const int detailLevel)
    {
       CreateUnitSphereHelper(vertices,  Vector3::AxisX(),  Vector3::AxisY(),  Vector3::AxisZ(), detailLevel);
       CreateUnitSphereHelper(vertices,  Vector3::AxisY(), -Vector3::AxisX(),  Vector3::AxisZ(), detailLevel);
       CreateUnitSphereHelper(vertices,  Vector3::AxisX(),  Vector3::AxisX(), -Vector3::AxisZ(), detailLevel);
       CreateUnitSphereHelper(vertices, -Vector3::AxisY(),  Vector3::AxisY(), -Vector3::AxisZ(), detailLevel);
    }

    template <typename VertexArray>
    void GetTrianglesContextVertexList::CreateHalfUnitSphereBottom(VertexArray& vertices, const int detailLevel)
    {
        CreateUnitSphereHelper(vertices, -Vector3::AxisX(), -Vector3::AxisY(),  Vector3::AxisZ(), detailLevel);
        CreateUnitSphereHelper(vertices, -Vector3::AxisY(),  Vector3::AxisX(),  Vector3::AxisZ(), detailLevel);
        CreateUnitSphereHelper(vertices,  Vector3::AxisX(), -Vector3::AxisY(), -Vector3::AxisZ(), detailLevel);
        CreateUnitSphereHelper(vertices, -Vector3::AxisY(), -Vector3::AxisX(), -Vector3::AxisZ(), detailLevel);
    }

    template <typename VertexArray>
    void GetTrianglesContextVertexList::CreateUnitOpenCylinder(VertexArray& vertices, const int detailLevel)
    {
        constexpr Vector3 bottomOffset(0.0f, -2.0f, 0.0f);
        int numVerts = 4 * (1 << detailLevel);
        for (int i = 0; i < numVerts; ++i)
        {
            const float angle1 = 2.0f * math::Pi<float>() * (static_cast<float>(i) / static_cast<float>(numVerts));
            const float angle2 = 2.0f * math::Pi<float>() * (static_cast<float>(i + 1) / static_cast<float>(numVerts));

            Vector3 t1(std::sin(angle1), 1.0f, std::cos(angle1));
            Vector3 t2(std::sin(angle2), 1.0f, std::cos(angle2));
            Vector3 b1 = t1 + bottomOffset;
            Vector3 b2 = t2 + bottomOffset;

            vertices.push_back(t1);
            vertices.push_back(b1);
            vertices.push_back(t2);

            vertices.push_back(t2);
            vertices.push_back(b1);
            vertices.push_back(b2);
        }
    }

    template <typename VertexArray>
    void GetTrianglesContextVertexList::CreateUnitSphereHelper(VertexArray& vertices, const Vector3& inV1,
        const Vector3& inV2, const Vector3& inV3, const int detailLevel)
    {
        Vector3 center1 = (inV1 + inV2).Normalized();
        Vector3 center2 = (inV2 + inV3).Normalized();
        Vector3 center3 = (inV3 + inV1).Normalized();

        if (detailLevel > 0)
        {
            const int newLevel = detailLevel - 1;
            CreateUnitSphereHelper(vertices, inV1, center1, center3, newLevel);
            CreateUnitSphereHelper(vertices, center1, center2, center3, newLevel);
            CreateUnitSphereHelper(vertices, center1, inV2, center2, newLevel);
            CreateUnitSphereHelper(vertices, center3, center2, inV3, newLevel);
        }
        else
        {
            vertices.push_back(inV1);
            vertices.push_back(inV2);
            vertices.push_back(inV3);
        }
    }
}