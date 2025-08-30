// ClipPoly.h
#pragma once
#include "AABox.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Clip a polygon against the positive halfspace of the plane defined by 'planeOrigin' and 'planeNormal'.
    ///     'planeNormal' does not need to be normalized.
    ///	@tparam VertexArrayType : Array type should have the std api format similar to std::vector.
    //----------------------------------------------------------------------------------------------------
    template <typename VertexArrayType>
    void ClipPolyVsPlane(const VertexArrayType& polygonToClip, const Vec3 planeOrigin, const Vec3 planeNormal, VertexArrayType& outClippedPolygon)
    {
        NES_ASSERT(polygonToClip.size() >= 2);
        NES_ASSERT(outClippedPolygon.empty());

        // Determine the state of the last point.
        Vec3 e1 = polygonToClip[polygonToClip.size() - 1];
        float prevNumerator = (planeOrigin - e1).Dot(planeNormal);
        bool prevIsInside = prevNumerator < 0.f;

        // Loop through all vertices
        for (typename VertexArrayType::size_type i = 0; i < polygonToClip.size(); ++i)
        {
            // Check if second point is inside
            const Vec3 e2 = polygonToClip[i];
            float numerator = (planeOrigin - e2).Dot(planeNormal);
            bool currIsInside = numerator < 0.f;

            // In -> Out or Out -> In: Add point on the clipping plane
            if (currIsInside != prevIsInside)
            {
                // Solve: (X - planeOrigin) . planeNormal = 0 and X = e1 + t * (e2 - e1) for X.
                Vec3 e12 = e2 - e1;
                float denominator = e12.Dot(planeNormal);
                if (denominator != 0.f)
                    outClippedPolygon.push_back(e1 + (prevNumerator / denominator) * e12);
                else
                    currIsInside = prevIsInside; // The edge is parallel to the plane, treat point as if it were on the same side as the last point.
            }

            // Point is inside, add it.
            if (currIsInside)
                outClippedPolygon.push_back(e2);

            // Update previous state
            prevNumerator = numerator;
            prevIsInside = currIsInside;
            e1 = e2;
        }
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Clip polygon versus polygon.
    ///	@tparam VertexArrayType : Array type should have the std api format similar to std::vector.
    ///	@param polygonToClip : The polygon that is being clipped.
    ///	@param clippingPolygon : The polygon which polygonToClip is clipped against.
    ///	@param clippingPolygonNormal : Used to create planes of all edges in clippingPolygon against which
    ///     polygonToClip is clipped. Does not need to be normalized.
    ///	@param outClippedPolygon : This will contain the clipped polygon when the function returns.
    //----------------------------------------------------------------------------------------------------
    template <typename VertexArrayType>
    void ClipPolyVsPoly(const VertexArrayType& polygonToClip, const VertexArrayType& clippingPolygon, const Vec3 clippingPolygonNormal, VertexArrayType& outClippedPolygon)
    {
        NES_ASSERT(polygonToClip.size() >= 2);
        NES_ASSERT(clippingPolygon.size() >= 3);

        VertexArrayType tempVertices[2];
        int tempVertexIndex = 0;

        for (typename VertexArrayType::size_type i = 0; i < clippingPolygon.size(); ++i)
        {
            // Get the edge to clip against.
            const Vec3 clipE1 = clippingPolygon[i];
            const Vec3 clipE2 = clippingPolygon[(i + 1) % clippingPolygon.size()];
            const Vec3 clipNormal = clippingPolygonNormal.Cross(clipE2 - clipE1); // Pointing inward to the clipping polygon.

            // Get the source and target polygon
            const VertexArrayType& sourcePolygon = (i == 0)? polygonToClip : tempVertices[tempVertexIndex];
            tempVertexIndex ^= 1;
            VertexArrayType& targetPolygon = (i == clippingPolygon.size() - 1)? outClippedPolygon : tempVertices[tempVertexIndex];
            targetPolygon.clear();

            // Clip against the edge
            ClipPolyVsPlane(sourcePolygon, clipE1, clipNormal, targetPolygon);

            // Break out if no polygon left
            if (targetPolygon.size() < 3)
            {
                outClippedPolygon.clear();
                break;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Clip 'polygonToClip' against an edge; the edge is projected on 'polygonToClip' using the
    ///     'clippingEdgeNormal'. The positive half-space (the side on the edge in the direction of the
    ///     'clippingEdgeNormal') is cut away.
    ///	@tparam VertexArrayType : Array type should have the std api format similar to std::vector.
    //----------------------------------------------------------------------------------------------------
    template <typename VertexArrayType>
    void ClipPolyVsEdge(const VertexArrayType& polygonToClip, const Vec3 edgeVertex1, const Vec3 edgeVertex2, const Vec3 clippingEdgeNormal, VertexArrayType& outClippedPolygon)
    {
        NES_ASSERT(polygonToClip.size() >= 3);
        NES_ASSERT(outClippedPolygon.empty());

        // Get the normal that is perpendicular to the edge and the clipping edge normal.
        Vec3 edge = edgeVertex2 - edgeVertex1;
        Vec3 edgeNormal = clippingEdgeNormal.Cross(edge);

        // Project vertices of the edge onto the polygon to clip
        Vec3 polygonNormal = (polygonToClip[2] - polygonToClip[0]).Cross(polygonToClip[1] - polygonToClip[0]);
        const float polygonNormalLengthSqr = polygonNormal.LengthSqr();
        Vec3 v1 = edgeVertex1 + polygonNormal.Dot(polygonToClip[0] - edgeVertex1) * polygonNormal / polygonNormalLengthSqr;
        Vec3 v2 = edgeVertex2 + polygonNormal.Dot(polygonToClip[0] - edgeVertex2) * polygonNormal / polygonNormalLengthSqr;
        Vec3 v12 = v2 - v1;
        const float v12LengthSqr = v12.LengthSqr();

        // Determine the state of the last point.
        Vec3 e1 = polygonToClip[polygonToClip.size() - 1];
        float prevNumerator = (edgeVertex1 - e1).Dot(edgeNormal);
        bool prevIsInside = prevNumerator < 0.f;

        // Loop through all vertices
        for (typename VertexArrayType::size_type i = 0; i < polygonToClip.size(); ++i)
        {
            // Check if the second point is inside
            Vec3 e2 = polygonToClip[i];
            const float numerator = (edgeVertex1 - e2).Dot(edgeNormal);
            const bool currIsInside = numerator < 0.f;

            // In -> Out or Out -> In: Add point on the clipping plane
            if (currIsInside != prevIsInside)
            {
                // Solve: (edgeVertex1 - X) . edgeNormal = 0 and X = e1 + t * (e2 - e1) for X.
                Vec3 e12 = e2 - e1;
                const float denominator = e12.Dot(edgeNormal);
                Vec3 clippedPoint = denominator != 0.f? e1 + (prevNumerator / denominator) * e12 : e1;

                // Project point onto the line segment v1, v2 to see if it falls outside the edge.
                const float projection = (clippedPoint - v1).Dot(v12);
                if (projection < 0.f)
                    outClippedPolygon.push_back(v1);
                else if (projection > v12LengthSqr)
                    outClippedPolygon.push_back(v2);
                else
                    outClippedPolygon.push_back(clippedPoint);
            }

            // Update the previous state
            prevNumerator = numerator;
            prevIsInside = currIsInside;
            e1 = e2;
        }
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Clip a polygon vs. an axis aligned box. 'polygonToClip' is assumed to be in counter-clockwise
    ///     order. Output will be stored in outClippedPolygon. Everything inside the box will be kept.
    ///	@tparam VertexArrayType : Array type should have the std api format similar to std::vector.
    //----------------------------------------------------------------------------------------------------
    template <typename VertexArrayType>
    void ClipPolyVsAABox(const VertexArrayType& polygonToClip, const AABox& box, VertexArrayType& outClippedPolygon)
    {
        NES_ASSERT(polygonToClip.size() >= 2);

        VertexArrayType tempVertices[2];
        int tempVertexIndex = 0;

        for (int coord = 0; coord < 3; ++coord)
        {
            for (int side = 0; side < 2; ++side)
            {
                // Get the plane to clip against.
                Vec3 origin = Vec3::Zero();
                Vec3 normal = Vec3::Zero();
                if (side == 0)
                {
                    normal[coord] = 1.f;
                    origin[coord] = box.m_min[coord];
                }
                else
                {
                    normal[coord] = -1.f;
                    origin[coord] = box.m_max[coord];
                }

                // Get the source and target polygon.
                const VertexArrayType& sourcePolygon = tempVertexIndex == 0? polygonToClip : tempVertices[tempVertexIndex & 1];
                ++tempVertexIndex;
                VertexArrayType& targetPolygon = tempVertexIndex == 0? outClippedPolygon : tempVertices[tempVertexIndex & 1];
                targetPolygon.clear();

                // Clip against the edge
                ClipPolyVsPlane(sourcePolygon, origin, normal, targetPolygon);

                // Break out if no polygon left
                if (targetPolygon.size() < 3)
                {
                    outClippedPolygon.clear();
                    return;
                }

                // Flip the normal
                normal = -normal;
            }
        }
    }
}
