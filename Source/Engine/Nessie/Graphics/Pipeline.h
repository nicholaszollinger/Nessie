// Pipeline.h
#pragma once
#include "DeviceObject.h"
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
    class Pipeline
    {
    public:
        Pipeline(std::nullptr_t) {}
        Pipeline(const Pipeline&) = delete;
        Pipeline(Pipeline&& other) noexcept;
        Pipeline& operator=(std::nullptr_t);
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline& operator=(Pipeline&& other) noexcept;
        ~Pipeline();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a Graphics Pipeline. 
        //----------------------------------------------------------------------------------------------------
        Pipeline(RenderDevice& device, PipelineLayout& layout, const GraphicsPipelineDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this Pipeline. 
        //----------------------------------------------------------------------------------------------------
        void                        SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Defines the binding type for the Pipeline. Can be Graphics, Compute, RayTracing, etc.
        //----------------------------------------------------------------------------------------------------
        vk::PipelineBindPoint       GetBindPoint() const { return m_bindPoint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Pipeline object.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::Pipeline&   GetVkPipeline() const { return m_pipeline; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject              GetNativeVkObject() const;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the Pipeline object. 
        //----------------------------------------------------------------------------------------------------
        void                        CreateGraphicsPipeline(const RenderDevice& device, PipelineLayout& layout, const GraphicsPipelineDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the resource to the Renderer to be freed.
        //----------------------------------------------------------------------------------------------------
        void                        FreePipeline();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initializes a shader stage's create info and shader module based on the shader description. 
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             SetupShaderStage(const ShaderDesc& desc, vk::PipelineShaderStageCreateInfo& outStage, vk::raii::ShaderModule& outModule);
        
    private:
        RenderDevice*               m_pDevice = nullptr;
        vk::raii::Pipeline          m_pipeline = nullptr;                           // Pipeline resource.
        vk::PipelineBindPoint       m_bindPoint = vk::PipelineBindPoint::eGraphics; // What stages the pipeline should be bound for use.
    };

    static_assert(DeviceObjectType<Pipeline>);
}
