// MotionProperties.cpp
#include "MotionProperties.h"

#include "MassProperties.h"

namespace nes
{
    void MotionProperties::SetMassProperties(const AllowedDOFs allowedDoFs, const MassProperties& massProperties)
    {
        // Store allowed DOFs
        m_allowedDoFs = allowedDoFs;

        // Decompose DOFs
        uint32_t allowedTranslationAxis = static_cast<uint32_t>(allowedDoFs) & 0b111; 
        uint32_t allowedRotationAxis = (static_cast<uint32_t>(allowedDoFs) >> 3) & 0b111;

        // Set inverse mass
        if (allowedTranslationAxis == 0)
        {
            m_inverseMass = 0.f;
        }
        else
        {
            NES_ASSERTV(massProperties.m_mass > 0.f, "Invalid Mass."
                "Some shapes like Mesh Shapes or Triangle Shape cannot calculate Mass automatically."
                "In this case, you need to provide it by setting BodyCreateInfo::m_overrideMassProperties and m_massPropertiesOverride.");

            m_inverseMass = 1.0f / massProperties.m_mass;
        }

        if (allowedRotationAxis == 0)
        {
            // No rotation possible.
            m_inverseInertiaDiagonal = Vector3::Zero();
            m_inertiaRotation = Quat::Identity();
        }
        else
        {
            // Set inverse inertia
            Mat4 rotation;
            Vector3 diagonal;
            if (massProperties.DecomposePrincipalMomentsOfInertia(rotation, diagonal)
                && !diagonal.IsNearZero())
            {
                m_inverseInertiaDiagonal = diagonal.GetReciprocal();
                m_inertiaRotation = math::ToQuat(rotation);
            }
            else
            {
                // Failed! Fall back ot inertia tensor of sphere with radius 1.
                m_inverseInertiaDiagonal = Vector3::Replicate(2.5f * m_inverseMass);
                m_inertiaRotation = Quat::Identity();
            }
        }

        NES_ASSERTV(m_inverseMass != 0.0f || m_inverseInertiaDiagonal != Vector3::Zero(), "Can't lock all axes, use a static body for this. This will crash with a division by zero later!");
    }

}
