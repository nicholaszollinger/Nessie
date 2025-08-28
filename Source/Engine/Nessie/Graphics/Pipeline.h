// Pipeline.h
#pragma once
#include "DeviceAsset.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Pipeline defines how input data is processed on the GPU. Pipelines can be bound at different
    ///     points in the execution, depending on its use. For instance, a graphics pipeline is a sequence of
    ///     operations that take vertices and textures of meshes all the way to pixels in the render targets.
    ///
    ///     See the PipelineDesc parameters for the Init functions for more information on how to build
    ///     a pipeline to suit your needs.
    ///
    /// @see : https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
    //----------------------------------------------------------------------------------------------------
    class Pipeline final : public DeviceAsset
    {
    public:
        explicit                    Pipeline(RenderDevice& device) : DeviceAsset(device) {}
        virtual                     ~Pipeline() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initializes this Pipeline as a Graphics Pipeline. 
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             Init(PipelineLayout& layout, const GraphicsPipelineDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this Pipeline. 
        //----------------------------------------------------------------------------------------------------
        virtual void                SetDebugName(const std::string& name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Defines the binding type for the Pipeline. Can be Graphics, Compute, RayTracing, etc.
        //----------------------------------------------------------------------------------------------------
        vk::PipelineBindPoint       GetBindPoint() const { return m_bindPoint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Pipeline object.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::Pipeline&   GetVkPipeline() const { return m_pipeline; }

    private:
        EGraphicsResult             SetupShaderStage(vk::PipelineShaderStageCreateInfo& outStage, const ShaderDesc& desc, vk::raii::ShaderModule& outModule);
        
    private:
        vk::raii::Pipeline          m_pipeline = nullptr;                           // Pipeline resource.
        vk::PipelineBindPoint       m_bindPoint = vk::PipelineBindPoint::eGraphics; // What stages the pipeline should be bound for use.
    };
}
