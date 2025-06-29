// MassProperties.cpp
#include "MassProperties.h"

#include <vulkan/vk_enum_string_helper.h>

#include "Core/InsertionSort.h"
#include "Math/Detail/EigenValueSymmetric.h"

namespace nes
{
    bool MassProperties::DecomposePrincipalMomentsOfInertia(Mat44& outRotation, Vec3& outDiagonal) const
    {
        // Using eigen decomposition to get the principal components of the inertia tensor
        // See: https://en.wikipedia.org/wiki/Eigendecomposition_of_a_matrix
        const Mat33 inertia = Mat33
        (
            m_inertia.GetColumn3(0),
            m_inertia.GetColumn3(1),
            m_inertia.GetColumn3(2)
        );
        
        Mat33 eigenVec = Mat33::Identity();
        Vec3 eigenValue;
        
        if (!EigenValueSymmetric(inertia, eigenVec, eigenValue))
            return false;

        // Sort so that the biggest value goes first.
        int indices[] = { 0, 1, 2 };
        InsertionSort(indices, indices + 3, [&eigenValue](const int left, const int right)
        {
            return eigenValue[left] > eigenValue[right]; 
        });

        // Convert to a regular Mat44 and Vec3
        outRotation = Mat44::Identity();
        for (int i = 0; i < 3; ++i)
        {
            outRotation.SetColumn3(i, eigenVec[indices[i]]);
            outDiagonal[i] = eigenValue[indices[i]];
        }

        // Make sure the result is left-handed.
        if (!Vec3::IsLeftHanded(outRotation.GetAxisX(), outRotation.GetAxisY(), outRotation.GetAxisZ()))
             outRotation[2] = -outRotation[2];

#if NES_ASSERTS_ENABLED
        // Validate that the solution is correct, for each axis we want to make sure that the different in inertia is
        // smaller than some fraction of the inertia itself in that axis.
        const Mat44 newInertia = outRotation * Mat44::MakeScale(outDiagonal) * outRotation.Inversed();
        for (int i = 0; i < 3; ++i)
        {
            NES_ASSERT(newInertia.GetColumn3(i).IsClose(m_inertia.GetColumn3(i), m_inertia.GetColumn3(i).LengthSqr() * 1.0e-10f));
        }
#endif
        
        return true;
    }

    void MassProperties::SetMassAndInertiaOfSolidBox(const Vec3& boxSize, const float density)
    {
        m_mass = boxSize.x * boxSize.y * boxSize.z * density;

        // Calculate inertia
        const Vec3 sizeSqr = boxSize * boxSize;
        const Vec3 scale = (sizeSqr.Swizzle<ESwizzleY, ESwizzleX, ESwizzleX>() + sizeSqr.Swizzle<ESwizzleZ, ESwizzleZ, ESwizzleY>()) * (m_mass / 12.f);
        m_inertia = Mat44::MakeScale(scale);
    }

    void MassProperties::ScaleToMass(const float mass)
    {
        if (m_mass > 0.f)
        {
            // Calculate how much we have to scale the inertia tensor:
            float massScale = mass / m_mass;

            // Update mass
            m_mass = mass;

            // Update the inertia tensor
            for (int i = 0; i < 3; ++i)
            {
                m_inertia[3][i] = m_inertia[3][i] * massScale;
            }
        }

        else
        {
            // Otherwise, just set the mass.
            m_mass = mass;
        }
    }

    void MassProperties::Rotate(const Mat44& rotation)
    {
        m_inertia = rotation.Multiply3x3(m_inertia).Multiply3x3RightTransposed(rotation);
    }

    void MassProperties::Translate(const Vec3& translation)
    {
        // Transform the inertia using the parallel axis theorem: I' = I + m * (translation^2 E - translation translation^T)
        // Where I is the original body's inertia and E the identity matrix
        // See: https://en.wikipedia.org/wiki/Parallel_axis_theorem
        m_inertia += (Mat44::MakeScale(translation.Dot(translation)) - Mat44::OuterProduct(translation, translation)) * m_mass;

        // Ensure that inertia is a 3x3 matrix, adding inertia causes the bottom right element to change
        m_inertia.SetColumn4(3, Vec4(0.f, 0.f, 0.f, 1.f));
    }

    void MassProperties::Scale(const Vec3& scale)
    {
        // See: https://en.wikipedia.org/wiki/Moment_of_inertia#Inertia_tensor
        // The diagonal of the inertia tensor can be calculated like this:
        // Ixx = sum_{k = 1 to n}(m_k * (y_k^2 + z_k^2))
        // Iyy = sum_{k = 1 to n}(m_k * (x_k^2 + z_k^2))
        // Izz = sum_{k = 1 to n}(m_k * (x_k^2 + y_k^2))
        //
        // We want to isolate the terms x_k, y_k and z_k:
        // d = [0.5, 0.5, 0.5].[Ixx, Iyy, Izz]
        // [sum_{k = 1 to n}(m_k * x_k^2), sum_{k = 1 to n}(m_k * y_k^2), sum_{k = 1 to n}(m_k * z_k^2)] = [d, d, d] - [Ixx, Iyy, Izz]
        const Vec3 diagonal = m_inertia.GetDiagonal3();
        const Vec3 xyzSqr = Vec3::Replicate(Vec3::Replicate(0.5f).Dot(diagonal)) - diagonal;

        // When scaling a shape these terms change like this:
        // sum_{k = 1 to n}(m_k * (scale_x * x_k)^2) = scale_x^2 * sum_{k = 1 to n}(m_k * x_k^2)
        // Same for y_k and z_k
        // Using these terms we can calculate the new diagonal of the inertia tensor:
        const Vec3 xyzScaledSqr = scale * scale * xyzSqr;
        const float iXX = xyzScaledSqr.y + xyzScaledSqr.z; 
        const float iYY = xyzScaledSqr.x + xyzScaledSqr.z; 
        const float iZZ = xyzScaledSqr.x + xyzScaledSqr.y;

        // The off diagonal elements are calculated like:
        // Ixy = -sum_{k = 1 to n}(x_k y_k)
        // Ixz = -sum_{k = 1 to n}(x_k z_k)
        // Iyz = -sum_{k = 1 to n}(y_k z_k)
        // Scaling these are simple:
        const float iXY = scale.x * scale.y * m_inertia[1][0];
        const float iXZ = scale.x * scale.z * m_inertia[2][0];
        const float iYZ = scale.y * scale.z * m_inertia[2][1];

        // Update inertia tensor
        m_inertia[0][0] = iXX;
        m_inertia[1][0] = iXY;
        m_inertia[0][1] = iXY;
        m_inertia[1][1] = iYY;
        m_inertia[2][0] = iXZ;
        m_inertia[0][2] = iXZ;
        m_inertia[2][1] = iYZ;
        m_inertia[1][2] = iYZ;
        m_inertia[2][2] = iZZ;

        // Mass scales linear with volume (Note that the scaling can be negative, and we don't want the mass to become negative).
        const float massScale = math::Abs(scale.x * scale.y * scale.z);
        m_mass *= massScale;

        // Inertia scales linear with mass. This updates the m_k terms above.
        m_inertia *= massScale;

        // Ensure that the bottom right is 1 again:
        m_inertia[3][3] = 1.f;
    }

    Vec3 MassProperties::GetEquivalentSolidBoxSize(const float mass, const Vec3& inertiaDiagonal)
    {
        // Moment of inertia of a solid box has diagonal equal to:
        // mass / 12 * [size.y^2 + size.z^2, size.x^2 + size.z^2, size.x^2 + size.y^2]
        // Solving for size.x, size.y and size.z (since diagonal and mass are known.
        Vec3 diagonal = inertiaDiagonal * (12.f / mass);
        return Vec3
        (
            std::sqrt(0.5f * (-diagonal[0] + diagonal[1] + diagonal[2])),
            std::sqrt(0.5f * (diagonal[0] - diagonal[1] + diagonal[2])),
            std::sqrt(0.5f * (diagonal[0] + diagonal[1] - diagonal[2]))
        );
    }
}
