// LightSpace.h
#pragma once
#include "LightTypes.h"

namespace helpers
{
    struct alignas (64) CascadedShadowMapsUBO
    {
        static constexpr uint32 kMaxCascades = 4;
        
        nes::Mat44      m_cascadeViewProjMatrices[kMaxCascades];
        nes::Vec4       m_splitDepths{}; // x = first split depth, etc.
        uint32          m_numCascades;
        float           m_shadowBias = 0.005f; // Adjust to eliminate acne/peter-panning.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Parameters to generate cascaded shadow map data.
    //----------------------------------------------------------------------------------------------------
    struct GenShadowCascadesDesc
    {
        nes::Mat44      m_cameraView;               // Camera's view matrix.
        nes::Mat44      m_cameraProj;               // Camera's projection matrix.
        float           m_cameraNear;               // Near plane of the camera.
        float           m_cameraFar;                // Far plane of the camera.
        uint32          m_numCascades;              // Number of cascades to generate.
        float           m_splitLambda = 0.5f;       // Determines the blend between uniform (0) and logarithmic (1) cascade splits.  
        float           m_shadowMapResolution;      // Image size of the shadow map. Should be 2048, 4096, etc.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Calculates the light space view-projection matrices and split depths for each cascade per the
    ///     given directional light.
    //----------------------------------------------------------------------------------------------------
    inline CascadedShadowMapsUBO GenerateShadowCascadesForLight(const DirectionalLight& light, const GenShadowCascadesDesc& desc)
    {
        CascadedShadowMapsUBO csm;
        csm.m_numCascades = desc.m_numCascades;
        
        float cascadeSplits[CascadedShadowMapsUBO::kMaxCascades];

        const float clipRange = desc.m_cameraFar - desc.m_cameraNear;
        const float minZ = desc.m_cameraNear;
        const float maxZ = desc.m_cameraNear + clipRange;

        const float range = maxZ - minZ;
        const float ratio = maxZ / minZ;

        // Calculate split depths based on view camera frustum
        // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
        // This calculates the normalized split distance for the far plane.
        for (uint32 i = 0; i < csm.m_numCascades; ++i)
        {
            const float p = static_cast<float>((i + 1)) / static_cast<float>(csm.m_numCascades);
            const float log = minZ * std::pow(ratio, p);
            const float uniform = minZ + range * p;
            const float depth = desc.m_splitLambda * (log - uniform) + uniform;
            cascadeSplits[i] = (depth - desc.m_cameraNear) / clipRange;
        }
        
        const nes::Mat44 invCamera = (desc.m_cameraProj * desc.m_cameraView).Inversed();
        
        // Calculate orthographic projection matrix for each cascade
        float lastSplitDist = 0.0;
        for (uint32 i = 0; i < csm.m_numCascades; ++i)
        {
            float splitDist = cascadeSplits[i];

            nes::Vec3 frustumCorners[8] =
            {
                nes::Vec3(-1.0f,  1.0f, 0.0f),
                nes::Vec3( 1.0f,  1.0f, 0.0f),
                nes::Vec3( 1.0f, -1.0f, 0.0f),
                nes::Vec3(-1.0f, -1.0f, 0.0f),
                nes::Vec3(-1.0f,  1.0f,  1.0f),
                nes::Vec3( 1.0f,  1.0f,  1.0f),
                nes::Vec3( 1.0f, -1.0f,  1.0f),
                nes::Vec3(-1.0f, -1.0f,  1.0f),
            };

            // Project frustum corners into world space
            for (uint32 j = 0; j < 8; ++j)
            {
                nes::Vec4 invCorner = invCamera * nes::Vec4(frustumCorners[j], 1.0f);
                invCorner /= invCorner.w; // Perspective Divide
                frustumCorners[j] = nes::Vec3(invCorner.x, invCorner.y, invCorner.z);
            }

            // Clip corners to the split range.
            for (uint32_t j = 0; j < 4; ++j)
            {
                nes::Vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
                frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
            }

            // Get the frustum center
            nes::Vec3 frustumCenter = nes::Vec3(0.0f);
            for (uint32 j = 0; j < 8; ++j)
            {
                frustumCenter += frustumCorners[j];
            }
            frustumCenter /= 8.f;
            
            // Calculate an extent radius.
            float radius = 0.f;
            for (uint32 j = 0; j < 8; ++j)
            {
                const float distance = (frustumCorners[j] - frustumCenter).Length();
                radius = nes::math::Max(radius, distance);
            }
            radius = std::ceil(radius * 16.f) / 16.f;

            nes::Vec3 maxExtents = nes::Vec3(radius);
            nes::Vec3 minExtents = -maxExtents;

            const nes::Vec3 lightDir = light.m_direction;
            const nes::Mat44 lightView = nes::Mat44::LookAt(
                frustumCenter - (lightDir * radius), // Light Position
                frustumCenter, // Looking at the center of the frustum.
                nes::Vec3::Up());
            
            const nes::Mat44 lightProj = nes::Mat44::Orthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.f, maxExtents.z - minExtents.z);
            csm.m_splitDepths[i] = (desc.m_cameraNear + splitDist * clipRange);
            csm.m_cascadeViewProjMatrices[i] = lightProj * lightView;

            lastSplitDist = cascadeSplits[i];
        }
        
        return csm;
    }
}