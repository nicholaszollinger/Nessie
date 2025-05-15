// GetTrianglesContext.cpp
#include "GetTrianglesContext.h"

#include "ScaleHelpers.h"

namespace nes
{
    GetTrianglesContextVertexList::GetTrianglesContextVertexList(const Vector3& positionCOM, const Quat& rotation,
        const Vector3& scale, const Mat4& localTransform, const Vector3* triangleVertices, size_t numTriangleVertices)
            : m_localToWorld(math::MakeRotationTranslationMatrix(positionCOM, rotation) * math::MakeScaleMatrix(scale) * localTransform)
            , m_pTriangleVertices(triangleVertices)
            , m_numTriangleVertices(numTriangleVertices)
            , m_isInsideOut(ScaleHelpers::IsInsideOut(scale))
    {
        static_assert(sizeof(GetTrianglesContextVertexList) <= sizeof(Shape::GetTrianglesContext), "GetTrianglesContext is too small!");
        NES_ASSERT(math::IsAligned(this, alignof(GetTrianglesContextVertexList)));
        NES_ASSERT(numTriangleVertices % 3 == 0);
    }

    int GetTrianglesContextVertexList::GetTrianglesNext(int maxTrianglesRequested, Float3* pOutTriangleVertices)
    {
        NES_ASSERT(maxTrianglesRequested >= Shape::kGetTrianglesMinTrianglesRequested);

        int totalNumVertices = math::Min(maxTrianglesRequested * 3, static_cast<int>(m_numTriangleVertices - m_currentVertex));

        if (m_isInsideOut)
        {
            // Store triangles flipped
            for (const Vector3* pVertex = m_pTriangleVertices + m_currentVertex, *pEnd = pVertex + m_numTriangleVertices; pVertex < pEnd; pVertex += 3)
            {
                m_localToWorld.TransformPoint(pVertex[0]).StoreFloat3(pOutTriangleVertices++);
                m_localToWorld.TransformPoint(pVertex[2]).StoreFloat3(pOutTriangleVertices++);
                m_localToWorld.TransformPoint(pVertex[1]).StoreFloat3(pOutTriangleVertices++);
            }
        }
        else
        {
            // Store triangles
            for (const Vector3* pVertex = m_pTriangleVertices + m_currentVertex, *pEnd = pVertex + m_numTriangleVertices; pVertex < pEnd; pVertex += 3)
            {
                m_localToWorld.TransformPoint(pVertex[0]).StoreFloat3(pOutTriangleVertices++);
                m_localToWorld.TransformPoint(pVertex[1]).StoreFloat3(pOutTriangleVertices++);
                m_localToWorld.TransformPoint(pVertex[2]).StoreFloat3(pOutTriangleVertices++);
            }
        }

        // Update the current vertex to point to the next vertex to get
        m_currentVertex += totalNumVertices;
        const int totalNumTriangles = totalNumVertices / 3;

        // [TODO]: Store materials

        return totalNumTriangles;
    }

    GetTrianglesContextMultiVertexList::GetTrianglesContextMultiVertexList(const bool isInsideOut)
        : m_isInsideOut(isInsideOut)
    {
        static_assert(sizeof(GetTrianglesContextMultiVertexList) <= sizeof(Shape::GetTrianglesContext), "GetTrianglesContext too small");
        NES_ASSERT(math::IsAligned(this, alignof(GetTrianglesContextMultiVertexList)));
    }

    void GetTrianglesContextMultiVertexList::AddPart(const Mat4& localToWorld, const Vector3* pTriangleVertices, const size_t numTriangleVertices)
    {
        NES_ASSERT(numTriangleVertices % 3 == 0);
        m_parts.push_back({ localToWorld, pTriangleVertices, numTriangleVertices});
    }

    int GetTrianglesContextMultiVertexList::GetTrianglesNext(int maxTrianglesRequested, Float3* pOutTriangleVertices)
    {
        NES_ASSERT(maxTrianglesRequested >= Shape::kGetTrianglesMinTrianglesRequested);

        int totalNumVertices = 0;
        int maxVerticesRequested = maxTrianglesRequested * 3;

        // Loop over parts
        for (; m_currentPart < m_parts.size(); ++m_currentPart)
        {
            const Part& part = m_parts[m_currentPart];

            // Calculate how many vertices to take from this part
            const int partNumVertices = std::min(maxVerticesRequested, static_cast<int>(part.m_numTriangleVertices - m_currentVertex));
            if (partNumVertices == 0)
                break;

            maxVerticesRequested -= partNumVertices;
            totalNumVertices += partNumVertices;

            if (m_isInsideOut)
            {
                // Store triangles flipped
                for (const Vector3* pVertex = part.m_pTriangleVertices + m_currentVertex, *pEnd = pVertex + part.m_numTriangleVertices; pVertex < pEnd; pVertex += 3)
                {
                    part.m_localToWorld.TransformPoint(pVertex[0]).StoreFloat3(pOutTriangleVertices++);
                    part.m_localToWorld.TransformPoint(pVertex[2]).StoreFloat3(pOutTriangleVertices++);
                    part.m_localToWorld.TransformPoint(pVertex[1]).StoreFloat3(pOutTriangleVertices++);
                }
            }
            else
            {
                // Store triangles flipped
                for (const Vector3* pVertex = part.m_pTriangleVertices + m_currentVertex, *pEnd = pVertex + part.m_numTriangleVertices; pVertex < pEnd; pVertex += 3)
                {
                    part.m_localToWorld.TransformPoint(pVertex[0]).StoreFloat3(pOutTriangleVertices++);
                    part.m_localToWorld.TransformPoint(pVertex[1]).StoreFloat3(pOutTriangleVertices++);
                    part.m_localToWorld.TransformPoint(pVertex[2]).StoreFloat3(pOutTriangleVertices++);
                }
            }

            // Update the current vertex to point to the next vertex to get.
            m_currentVertex += partNumVertices;

            // Check if we've completed this part
            if (m_currentVertex < part.m_numTriangleVertices)
                break;

            // Reset current vertex for next part.
            m_currentVertex = 0;
        }

        const int totalNumTriangles = totalNumVertices / 3;

        // [TODO]: Store materials

        return totalNumTriangles;
    }
}
