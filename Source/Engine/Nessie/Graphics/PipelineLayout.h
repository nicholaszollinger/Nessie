// PipelineLayout.h
#pragma once
#include "DeviceObject.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Pipeline Layout defines the uniform and push constant variables referenced in the shaders
    ///     that can be updated at draw time. A single pipeline layout can be used for different pipelines,
    ///     as long as the shader variables are the same.
    //----------------------------------------------------------------------------------------------------
    class PipelineLayout
    {
    public:
        PipelineLayout(std::nullptr_t) {}
        PipelineLayout(const PipelineLayout&) = delete;
        PipelineLayout(PipelineLayout&& other) noexcept;
        PipelineLayout& operator=(std::nullptr_t);
        PipelineLayout& operator=(const PipelineLayout&) = delete;
        PipelineLayout& operator=(PipelineLayout&& other) noexcept;
        ~PipelineLayout();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the Pipeline layout resource.
        //----------------------------------------------------------------------------------------------------
        PipelineLayout(RenderDevice& device, const PipelineLayoutDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this resource.
        //----------------------------------------------------------------------------------------------------
        void                        SetDebugName(const std::string& name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the binding point for this pipeline. Can be graphics, compute, or ray tracing.
        //----------------------------------------------------------------------------------------------------
        vk::PipelineBindPoint       GetBindPoint() const            { return m_bindPoint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan Pipeline Layout resource.
        //----------------------------------------------------------------------------------------------------
        vk::raii::PipelineLayout&   GetVkPipelineLayout()           { return m_layout; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject              GetNativeVkObject() const;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the pipeline layout. 
        //----------------------------------------------------------------------------------------------------
        void                        CreatePipelineLayout(const RenderDevice& device, const PipelineLayoutDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the resource to the Renderer to be freed.
        //----------------------------------------------------------------------------------------------------
        void                        FreeLayout();

    private:
        RenderDevice*           m_pDevice = nullptr;
        vk::raii::PipelineLayout    m_layout = nullptr;
        vk::PipelineBindPoint       m_bindPoint = vk::PipelineBindPoint::eGraphics;
    };

    static_assert(DeviceObjectType<PipelineLayout>);
}
