// PipelineLayout.h
#pragma once
#include "DeviceAsset.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Pipeline Layout defines the uniform and push constant variables referenced in the shaders
    ///     that can be updated at draw time. A single pipeline layout can be used for different pipelines,
    ///     as long as the shader variables are the same.
    //----------------------------------------------------------------------------------------------------
    class PipelineLayout final : public DeviceAsset
    {
    public:
        explicit                    PipelineLayout(RenderDevice& device) : DeviceAsset(device) {}
        virtual                     ~PipelineLayout() override;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the Pipeline layout resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             Init(const PipelineLayoutDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this resource.
        //----------------------------------------------------------------------------------------------------
        virtual void                SetDebugName(const std::string& name) override;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the binding point for this pipeline. Can be graphics, compute, or ray tracing.
        //----------------------------------------------------------------------------------------------------
        vk::PipelineBindPoint       GetBindPoint() const        { return m_bindPoint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan Pipeline Layout resource.
        //----------------------------------------------------------------------------------------------------
        vk::raii::PipelineLayout&   GetVkPipelineLayout()       { return m_layout; }

    private:
        vk::raii::PipelineLayout    m_layout = nullptr;
        vk::PipelineBindPoint       m_bindPoint = vk::PipelineBindPoint::eGraphics;
    };  
}
