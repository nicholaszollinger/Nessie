// CameraComponent.cpp
#include "CameraComponent.h"

#include "Nessie/FileIO/YAML/Serializers/YamlGraphicsSerializers.h"

namespace nes
{
    void CameraComponent::Serialize(YamlOutStream& out, const CameraComponent& component)
    {
        CameraSerializer::Serialize(out, component.m_camera);
    }

    void CameraComponent::Deserialize(const YamlNode& in, CameraComponent& component)
    {
        CameraSerializer::Deserialize(in, component.m_camera);
    }
}
