// YamlSerializers.cpp
#include "YamlSerializers.h"
#include "Nessie/Graphics/Camera.h"

NES_YAML_DEFINE_ENUM_CONVERTER(nes::Camera::EProjectionType);

namespace nes
{
    void CameraSerializer::Deserialize(const YamlNode& in, Camera& camera)
    {
        in["PerspectiveFOV"].Read(camera.m_perspectiveFOV, 60.f);
        in["OrthographicSize"].Read(camera.m_orthographicSize, 10.f);
        in["NearPlane"].Read(camera.m_nearPlane, 0.1f);
        in["FarPlane"].Read(camera.m_farPlane, 1000.f);
        in["Aperture"].Read(camera.m_aperture, 1.f);
        in["ShutterSpeed"].Read(camera.m_shutterSpeed, 125.f);
        in["ISO"].Read(camera.m_iso, 100.f);
        in["ProjectionType"].Read(camera.m_projectionType, Camera::EProjectionType::Perspective);
    }

    void CameraSerializer::Serialize(YamlWriter& out, const Camera& camera)
    {
        out.BeginMap("Camera");
        out.Write("PerspectiveFOV", camera.m_perspectiveFOV);
        out.Write("OrthographicSize", camera.m_orthographicSize);
        out.Write("NearPlane", camera.m_nearPlane);
        out.Write("FarPlane", camera.m_farPlane);
        out.Write("Aperture", camera.m_aperture);
        out.Write("ShutterSpeed", camera.m_shutterSpeed);
        out.Write("Iso", camera.m_iso);
        out.Write("ProjectionType", camera.m_projectionType);
        out.EndMap();
    }
}
