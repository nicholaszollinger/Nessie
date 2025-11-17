// YamlGraphicsSerializers.cpp
#include "YamlGraphicsSerializers.h"

namespace nes
{
    void CameraSerializer::Deserialize(const YamlNode& in, Camera& camera)
    {
        auto cameraNode = in["Camera"];
        if (!cameraNode.IsValid())
            return;
        
        cameraNode["PerspectiveFOV"].Read(camera.m_perspectiveFOV, 60.f);
        cameraNode["OrthographicSize"].Read(camera.m_orthographicSize, 10.f);
        cameraNode["NearPlane"].Read(camera.m_nearPlane, 0.1f);
        cameraNode["FarPlane"].Read(camera.m_farPlane, 1000.f);
        cameraNode["Aperture"].Read(camera.m_aperture, 8.f);
        cameraNode["ShutterSpeed"].Read(camera.m_shutterSpeed, 125.f);
        cameraNode["ISO"].Read(camera.m_iso, 100.f);
        cameraNode["ProjectionType"].Read(camera.m_projectionType, Camera::EProjectionType::Perspective);
    }

    void CameraSerializer::Serialize(YamlOutStream& out, const Camera& camera)
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
