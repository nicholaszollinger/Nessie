// InternalEdgeRemovingCollector.cpp
#include "InternalEdgeRemovingCollector.h"
#include "Core/QuickSort.h"

namespace nes
{
    InternalEdgeRemovingCollector::InternalEdgeRemovingCollector(CollideShapeCollector& chainedCollector)
        : CollideShapeCollector(chainedCollector)
        , m_chainedCollector(chainedCollector)
    {
        // Initialize arrays to reserve full capacity to avoid needless reallocation calls.
        m_voidedFeatures.reserve(kMaxLocalVoidedFeatures);
        m_delayedResults.reserve(kMaxLocalDelayedResults);
    }

    void InternalEdgeRemovingCollector::Reset()
    {
        CollideShapeCollector::Reset();

        m_chainedCollector.Reset();
        m_voidedFeatures.clear();
        m_delayedResults.clear();
    }

    void InternalEdgeRemovingCollector::OnBody(const Body& body)
    {
        // Forward the call to our chained collector.
        m_chainedCollector.OnBody(body);
    }

    void InternalEdgeRemovingCollector::OnBodyEnd()
    {
        Flush();
        m_chainedCollector.OnBodyEnd();
    }

    void InternalEdgeRemovingCollector::AddHit(const ResultType& result)
    {
        // We only support welding when the shape is a triangle or has more vertices so that we can calculate a normal.
        if (result.m_shape2Face.size() < 3)
            return ChainAndVoid(result);

        // Get the triangle normal of shape 2 face.
        const Vec3 triangleNormal = (result.m_shape2Face[1] - result.m_shape2Face[0]).Cross(result.m_shape2Face[2] - result.m_shape2Face[0]);
        const float triangleNormalLength = triangleNormal.Length();
        if (triangleNormalLength < 1e-6f)
            return ChainAndVoid(result);

        // If the triangle normal matches the contact normal within 1 degree, we can process the contact immediately.
        // We make the assumption here that if the contact normal and the triangle normal align that we're dealing with a face contact.
        const Vec3 contactNormal = -result.m_penetrationAxis;
        const float contactNormalLength = contactNormal.Length();
        if (triangleNormal.Dot(contactNormal) > 0.999848f * contactNormalLength * triangleNormalLength) // cos(1 degree)
            return ChainAndVoid(result);

        // Delayed processing
        m_delayedResults.push_back(result);
    }

    void InternalEdgeRemovingCollector::Flush()
    {
        // Sort on the biggest penetration depth first.
        std::vector<uint, STLLocalAllocator<uint, kMaxLocalDelayedResults>> sortedIndices;
        sortedIndices.reserve(m_delayedResults.size());
        for (uint i = 0; i < static_cast<uint>(m_delayedResults.size()); ++i)
        {
            sortedIndices[i] = i;
        }
        QuickSort(sortedIndices.begin(), sortedIndices.end(), [this](const uint left, const uint right)
        {
            return m_delayedResults[left].m_penetrationDepth > m_delayedResults[right].m_penetrationDepth;
        });

        // Loop over all results
        for (uint i = 0; i < static_cast<uint>(m_delayedResults.size()); ++i)
        {
            const CollideShapeResult& result = m_delayedResults[sortedIndices[i]];

            // Determine which vertex of which edge is the closest to the contact point.
            float bestDistSqr = FLT_MAX;
            uint bestV1Index = 0;
            uint bestV2Index = 0;
            const uint numVertices = static_cast<uint>(result.m_shape2Face.size());
            uint v1Index = numVertices -1;
            Vec3 v1 = result.m_shape2Face[v1Index] - result.m_contactPointOn2;
            for (uint v2Index = 0; v2Index < numVertices; ++v2Index)
            {
                const Vec3 v2 = result.m_shape2Face[v2Index] - result.m_contactPointOn2;
                const Vec3 v1v2 = v2 - v1;
                float denominator = v1v2.LengthSqr();
                if (denominator < math::Squared(FLT_EPSILON))
                {
                    // Degenerate, assume v1 is the closest; v2 will be tested in a later iteration.
                    const float v1LengthSqr = v1.LengthSqr();
                    if (v1LengthSqr < bestDistSqr)
                    {
                        bestDistSqr = v1LengthSqr;
                        bestV1Index = v1Index;
                        bestV2Index = v1Index;
                    }
                }
                else
                {
                    // Taken from ClosestPoint::GetBaryCentricCoordinates()
                    const float fraction = -v1.Dot(v1v2) / denominator;
                    if (fraction < 1.0e-6f)
                    {
                        // Closest lies on v1
                        const float v1LengthSqr = v1.LengthSqr();
                        if (v1LengthSqr < bestDistSqr)
                        {
                            bestDistSqr = v1LengthSqr;
                            bestV1Index = v1Index;
                            bestV2Index = v1Index;
                        }
                        else if (fraction < 1.f - 1.0e-6f)
                        {
                            // Closest lies on the line segment v1, v2
                            const Vec3 closest = v1 + fraction * v1v2;
                            const float closestLengthSqr = closest.LengthSqr();
                            if (closestLengthSqr < bestDistSqr)
                            {
                                bestDistSqr = closestLengthSqr;
                                bestV1Index = v1Index;
                                bestV2Index = v2Index;
                            }
                        }
                        // Else closest is v2, but v2 will be tested in a later iteration.
                    }
                }

                v1Index = v2Index;
                v1 = v2;
            }

            // Check if this vertex/edge is voided.
            bool voided = IsVoided(result.m_subShapeID1, result.m_shape2Face[bestV1Index])
                && (bestV1Index == bestV2Index || IsVoided(result.m_subShapeID1, result.m_shape2Face[bestV2Index]));

            // No voided features, accept the contact
            if (!voided)
                Chain(result);

            // Void the features of this face
            VoidFeatures(result);
        }

        // All delayed results have been processed
        m_delayedResults.clear();
        m_voidedFeatures.clear();
    }

    void InternalEdgeRemovingCollector::CollideShapeVsShape(const Shape* pShape1, const Shape* pShape2, const Vec3 scale1, const Vec3 scale2, const Mat44& centerOfMassTransform1, const Mat44& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, const CollideShapeSettings& settings, CollideShapeCollector& inCollector, const ShapeFilter& shapeFilter)
    {
        NES_ASSERT(settings.m_activeEdgeMode == EActiveEdgeMode::CollideWithAll); // Won't work without colliding with all edges.
        NES_ASSERT(settings.m_collectFacesMode == ECollectFacesMode::CollectFaces); // Won't work without collecting faces.

        InternalEdgeRemovingCollector wrapper(inCollector);
        CollisionSolver::CollideShapeVsShape(pShape1, pShape2, scale1, scale2, centerOfMassTransform1, centerOfMassTransform2, subShapeIDCreator1, subShapeIDCreator2, settings, wrapper, shapeFilter);
        wrapper.Flush();
    }

    inline bool InternalEdgeRemovingCollector::IsVoided(const SubShapeID& subShapeID, const Vec3 vec) const
    {
        for (const Voided& vf : m_voidedFeatures)
        {
            if (vf.m_subShapeID == subShapeID && vec.IsClose(Vec3::LoadFloat3Unsafe(vf.m_feature), 1.0e-8f))
            {
                return true;
            }
        }
        
        return false;
    }

    inline void InternalEdgeRemovingCollector::VoidFeatures(const CollideShapeResult& result)
    {
        for (const Vec3& v : result.m_shape2Face)
        {
            if (!IsVoided(result.m_subShapeID1, v))
            {
                Voided vf;
                v.StoreFloat3(&vf.m_feature);
                vf.m_subShapeID = result.m_subShapeID1;
                m_voidedFeatures.push_back(vf);
            }
        }
    }

    inline void InternalEdgeRemovingCollector::Chain(const CollideShapeResult& result)
    {
        // Make sure the chained collector has the same context as we do.
        m_chainedCollector.SetContext(GetContext());

        // Forward the hit.
        m_chainedCollector.AddHit(result);

        // If our chained collector updated its early out fraction, we need to follow.
        UpdateEarlyOutFraction(m_chainedCollector.GetEarlyOutFraction());
    }

    inline void InternalEdgeRemovingCollector::ChainAndVoid(const CollideShapeResult& result)
    {
        Chain(result);
        VoidFeatures(result);
    }
}
