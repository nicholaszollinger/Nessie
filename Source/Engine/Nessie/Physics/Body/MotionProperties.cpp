// MotionProperties.cpp
#include "MotionProperties.h"
#include "MassProperties.h"

namespace nes
{
    void MotionProperties::SetMassProperties(const EAllowedDOFs allowedDoFs, const MassProperties& massProperties)
    {
        // Store allowed DOFs
        m_allowedDoFs = allowedDoFs;

        // Decompose DOFs
        uint allowedTranslationAxis = static_cast<uint>(allowedDoFs) & 0b111; 
        uint allowedRotationAxis = (static_cast<uint>(allowedDoFs) >> 3) & 0b111;

        // Set inverse mass
        if (allowedTranslationAxis == 0)
        {
            m_inverseMass = 0.f;
        }
        else
        {
            NES_ASSERT(massProperties.m_mass > 0.f, "Invalid Mass."
                "Some shapes like Mesh Shapes or Triangle Shape cannot calculate Mass automatically."
                "In this case, you need to provide it by setting BodyCreateInfo::m_overrideMassProperties and m_massPropertiesOverride.");

            m_inverseMass = 1.0f / massProperties.m_mass;
        }

        if (allowedRotationAxis == 0)
        {
            // No rotation possible.
            m_inverseInertiaDiagonal = Vec3::Zero();
            m_inertiaRotation = Quat::Identity();
        }
        else
        {
            // Set inverse inertia
            Mat44 rotation;
            Vec3 diagonal;
            if (massProperties.DecomposePrincipalMomentsOfInertia(rotation, diagonal)
                && !diagonal.IsNearZero())
            {
                m_inverseInertiaDiagonal = diagonal.Reciprocal();
                m_inertiaRotation = rotation.ToQuaternion();
            }
            else
            {
                // Failed! Fall back ot inertia tensor of sphere with radius 1.
                m_inverseInertiaDiagonal = Vec3::Replicate(2.5f * m_inverseMass);
                m_inertiaRotation = Quat::Identity();
            }
        }

        NES_ASSERT(m_inverseMass != 0.0f || m_inverseInertiaDiagonal != Vec3::Zero(), "Can't lock all axes, use a static body for this. This will crash with a division by zero later!");
    }

}
