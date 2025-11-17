// CameraComponentInspector.cpp
#include "CameraComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"

namespace nes
{
    void CameraComponentInspector::DrawImpl(CameraComponent* pTarget, const InspectorContext&)
    {
        auto& camera = pTarget->m_camera;

        // Projection Type.
        using ProjectionTypeValue = editor::EnumPropertyValueDesc<Camera::EProjectionType>;
        using ProjectionTypeOptions = std::vector<ProjectionTypeValue>;
        static const ProjectionTypeOptions options =
        {
            ProjectionTypeValue
            {
                Camera::EProjectionType::Perspective,
                "Perspective",
                "Single-point perspective. This is the standard viewing for most 3D applications."
            },

            ProjectionTypeValue
            {
                Camera::EProjectionType::Orthographic,
                "Orthographic",
                "Preserves parallel lines. This is primarily used in 2D rendering, (HUDs, Menus, etc.) and isometric views."
            }   
        };
        
        editor::PropertyEnum<Camera::EProjectionType>("ProjectionType", camera.m_projectionType, options);
        
        // Perspective FOV or Orthographic Size, depending on Mode.
        if (camera.m_projectionType == Camera::EProjectionType::Perspective)
            editor::Property("Perspective FOV", camera.m_perspectiveFOV, 0.1f, 40.f, 180.f, "%.2f°", "The perspective field of view, expressed in degrees. Should be ~[45-120]");
        else
            editor::Property("Orthographic Size", camera.m_orthographicSize, 0.1f, 0.01f, FLT_MAX, "%.f", "Determines the height of the orthographic frustum. The width of the frustum is equal to 'half the size' * 'the aspect ratio of the viewport'.");

        if (editor::Property("Near Plane", camera.m_nearPlane, 0.1f, 0.001f, FLT_MAX, "%.3f", "The near plane distance from the view position. Anything closer than this position will be invisible. 0.1f is a reasonable default."))
        {
            camera.m_nearPlane = math::Min(camera.m_nearPlane, camera.m_farPlane);            
        }
        if (editor::Property("Far Plane", camera.m_farPlane, 0.1f, 0.001f, FLT_MAX, "%.3f", "The near plane distance from the view position. Anything closer than this position will be invisible. 0.1f is a reasonable default."))
        {
            camera.m_farPlane = math::Max(camera.m_nearPlane, camera.m_farPlane);            
        }

        // [TODO]: Have a slider that moves between these inputs:
        // Values: 1.0, 1.4, 2, 2.8, 4, 5.6, 8, 11, 16, 22
        editor::Property("Aperture", camera.m_aperture, 1.f, 1.f, 22.f, "%.1f", "AKA the 'f-stop'. Determines the size of the opening in the lens that allows light through."
            "\n- Lower numbers (1.4, 2.8) = wider aperture = more light."
            "\n- Higher numbers (8, 16) = narrower aperture = less light.");

        editor::Property("Shutter Speed", camera.m_shutterSpeed, 1.f, 0.f, FLT_MAX, "%.0f", "How long the camera sensor is exposed to light, in seconds. When calling CalculateExposureFactor, this value will be"
            "set over 1. So if you want a shutter speed of 1/500s, set this to 500."
             "\n- Fast Speed (1/500s, 1/1000s) = less light."
             "\n- Slow Speed (1/30s, 1) = more light.");

        editor::Property("ISO", camera.m_iso, 1.f, 1.f, FLT_MAX, "%.0f", "Sensor sensitivity. Acts as a linear brightness multiplier.");
    }
}
