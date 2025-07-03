// ManifoldBetweenTwoFaces.cpp

#include "Nessie/Physics/Collision/ManifoldBetweenTwoFaces.h"
#include "Nessie/Physics/Constraints/ContactConstraintManager.h"
#include "Nessie/Geometry/ClipPoly.h"

namespace nes
{
    void PruneContactPoints(const Vec3& penetrationAxis, ContactPoints& contactPointsOn1, ContactPoints& contactPointsOn2)
    {
        // Makes no sense to call this with 4 or less points.
        NES_ASSERT(contactPointsOn1.size() > 4);

        // Both arrays should have the same size.
        NES_ASSERT(contactPointsOn1.size() == contactPointsOn2.size());

        // Penetration axis must be normalized
        NES_ASSERT(penetrationAxis.IsNormalized());

        // We use a heuristic of (distance to center of mass) * (penetration depth) to find the contact point that we should keep.
        // Neither of these two terms should ever become 0, so we clamp against this minimum value.
        static constexpr float kMinDistanceSqr = 1.0e-6f; // 1 mm

        ContactPoints projected;
        StaticArray<float, 64> penetrationDepthSqr;
        for (ContactPoints::size_type i = 0; i < contactPointsOn1.size(); ++i)
        {
            // Project contact points on the plane through centerOfMass with normal penetration axis and center around the center of the mass of the body.
            // (note that since all points are relative to the centerOfMass, we can project onto the plane through the origin).
            Vec3 v1 = contactPointsOn1[i];
            projected.push_back(v1 - v1.Dot(penetrationAxis) * penetrationAxis);

            // Calculate penetration depth^2 of each point and clamp against the minimal distance.
            Vec3 v2 = contactPointsOn2[i];
            penetrationDepthSqr.push_back(math::Max(kMinDistanceSqr, (v2 - v1).LengthSqr()));
        }

        // Find the point that is furthest away from the center of mass (its torque will have the biggest influence)
        // and the point that has the deepest penetration depth. Use the heuristic (distance to the center of mass) * (penetration depth) for this.
        uint point1Index = 0;
        float val = math::Max(kMinDistanceSqr, projected[0].LengthSqr()) * penetrationDepthSqr[0];
        for (uint i = 1; i < projected.size(); ++i)
        {
            const float v = math::Max(kMinDistanceSqr, projected[i].LengthSqr()) * penetrationDepthSqr[i];
            if (v > val)
            {
                val = v;
                point1Index = i;
            }
        }
        Vec3 point1V = projected[point1Index];

        // Find the point furthest from the first point forming a line segment with point1. Again, combine this with the heuristic
        // for the deepest point as per above.
        uint point2Index = static_cast<uint>(-1);
        val = -FLT_MAX;
        for (uint i = 0; i < projected.size(); ++i)
        {
            if (i != point1Index)
            {
                const float v = math::Max(kMinDistanceSqr, (projected[i] - point1V).LengthSqr()) * penetrationDepthSqr[i];
                if (v > val)
                {
                    val = v;
                    point2Index = i;
                }
            }
        }
        NES_ASSERT(point2Index != static_cast<uint>(-1));
        Vec3 point2V = projected[point2Index];

        // Find the furthest points on both sides of the line segment to maximize the area.
        uint point3Index = static_cast<uint>(-1);
        uint point4Index = static_cast<uint>(-1);
        float minVal = 0.f;
        float maxVal = 0.f;
        const Vec3 perp = (point2V - point1V).Cross(penetrationAxis);
        for (uint i = 0; i < projected.size(); ++i)
        {
            if (i != point1Index && i != point2Index)
            {
                const float v = perp.Dot(projected[i] - point1V);
                if (v < minVal)
                {
                    minVal = v;
                    point3Index = i;
                }
                else if (v > maxVal)
                {
                    maxVal = v;
                    point4Index = i;
                }
            }
        }

        // Add points to the array (in order so they form polygon).
        StaticArray<Vec3, 4> pointsToKeepOn1;
        StaticArray<Vec3, 4> pointsToKeepOn2;
        pointsToKeepOn1.push_back(contactPointsOn1[point1Index]);
        pointsToKeepOn2.push_back(contactPointsOn2[point1Index]);
        if (point3Index != static_cast<uint>(-1))
        {
            pointsToKeepOn1.push_back(contactPointsOn1[point3Index]);
            pointsToKeepOn2.push_back(contactPointsOn2[point3Index]);
        }
        pointsToKeepOn1.push_back(contactPointsOn1[point2Index]);
        pointsToKeepOn2.push_back(contactPointsOn2[point2Index]);
        if (point4Index != static_cast<uint>(-1))
        {
            pointsToKeepOn1.push_back(contactPointsOn1[point4Index]);
            pointsToKeepOn2.push_back(contactPointsOn2[point4Index]);
        }

        // Copy the points back to the input buffer
        contactPointsOn1 = pointsToKeepOn1;
        contactPointsOn2 = pointsToKeepOn2;
    }

    void ManifoldBetweenTwoFaces(const Vec3& contactPoint1, const Vec3& contactPoint2, const Vec3& penetrationAxis, float maxContactDistance, const ConvexShape::SupportingFace& shape1Face, const ConvexShape::SupportingFace& shape2Face, ContactPoints& outContactPoints1, ContactPoints& outContactPoints2)
    {
        NES_ASSERT(maxContactDistance > 0.f);

        // Remember size before adding new points, to check at the end if we added some.
        const ContactPoints::size_type oldSize = outContactPoints1.size();

        // Check if both shapes have polygon faces
        if (shape1Face.size() >= 2      // The dynamic shape needs to have at least 2 points or else there can never be more than 1 contact point.
            && shape2Face.size() >= 3)  // The dynamic/static shape needs to have at least 3 points (in the case that it has 2 points only if edges if the edges match exactly you can have 2 contact points, but this situation is unstable anyhow). 
        {
            // Clip the polygon of face 2 against that of 1.
            ConvexShape::SupportingFace clippedFace;
            if (shape1Face.size() >= 3)
                ClipPolyVsPoly(shape2Face, shape1Face, penetrationAxis, clippedFace);
            else if (shape1Face.size() == 2)
                ClipPolyVsEdge(shape2Face, shape1Face[0], shape1Face[1], penetrationAxis, clippedFace);

            // Determine plane origin and normal of shape 1.
            Vec3 planeOrigin = shape1Face[0];
            Vec3 planeNormal;
            Vec3 firstEdge = shape1Face[1] - planeOrigin;
            if (shape1Face.size() >= 3)
            {
                // Three vertices, you can just calculate the normal
                planeNormal = firstEdge.Cross(shape1Face[2] - planeOrigin);
            }
            else
            {
                // Two vertices, first find a perpendicular to the edge and penetration axis, and then use the perpendicular together with the edge to form a normal.
                planeNormal = firstEdge.Cross(penetrationAxis).Cross(firstEdge);
            }

            // If penetration axis and plane normal are perpendicular, fall back to the contact points.
            const float penetrationAxisDotPlaneNormal = penetrationAxis.Dot(planeNormal);
            if (penetrationAxisDotPlaneNormal != 0.f)
            {
                const float penetrationAxisLength = penetrationAxis.Length();

                for (Vec3 p2 : clippedFace)
                {
                    // Project the clipped face back onto the plane of face 1. We do this by solving:
                    // p1 = p2 + distance * penetrationAxis / |penetrationAxis|.
                    // (p1 - planeOrigin) . planeNormal = 0
                    // This gives us:
                    // distance = -|penetrationAxis| * (p2 - planeOrigin) . planeNormal / penetrationAxis . planeNormal
                    const float distance = (p2 - planeOrigin).Dot(planeNormal) / penetrationAxisDotPlaneNormal; // Note: left out -|penetrationAxis| term.

                    // If the point is less than maxContactDistance in front of the plane of face 2, add it as a contact point.
                    if (distance * penetrationAxisLength < maxContactDistance)
                    {
                        Vec3 p1 = p2 - distance * penetrationAxis;
                        outContactPoints1.push_back(p1);
                        outContactPoints2.push_back(p2);
                    }
                }
            }
        }

        // If the clipping result is empty, use the contact point itself
        if (outContactPoints1.size() == oldSize)
        {
            outContactPoints1.push_back(contactPoint1);
            outContactPoints2.push_back(contactPoint2);
        }
    }
}
