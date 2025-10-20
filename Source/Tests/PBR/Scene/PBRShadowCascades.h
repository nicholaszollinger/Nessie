// PBRShadowCascades.h
#pragma once
#include "PBRLights.h"

namespace pbr
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

    namespace helpers
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the light space view-projection matrices and split depths for each cascade per the
        ///     given directional light.
        //----------------------------------------------------------------------------------------------------
        CascadedShadowMapsUBO GenerateShadowCascadesForLight(const DirectionalLight& light, const GenShadowCascadesDesc& desc);
    }
}