// PBRScene.cpp
#include "PBRScene.h"

namespace pbr
{
    InstanceUBO& InstanceUBO::SetTransform(const nes::Vec3 translation, const nes::Quat rotation, const nes::Vec3 scale)
    {
        m_model = nes::Mat44::ComposeTransform(translation, rotation, scale);
        m_normal = m_model.Inversed3x3().Transposed3x3();
        return *this;
    }

    InstanceUBO& InstanceUBO::SetTransform(const nes::Mat44& transform)
    {
        m_model = transform;
        m_normal = m_model.Inversed3x3().Transposed3x3();
        return *this;
    }

    InstanceUBO& InstanceUBO::SetMesh(const uint32 meshIndex)
    {
        m_meshIndex = meshIndex;
        return *this;
    }

    InstanceUBO& InstanceUBO::SetMaterial(const uint32 materialIndex)
    {
        m_materialIndex = materialIndex;
        return *this;
    }
}