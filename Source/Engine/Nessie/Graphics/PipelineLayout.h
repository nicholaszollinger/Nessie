// PipelineLayout.h
#pragma once
#include "DeviceObject.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //  Resources:
    //  - Mapping Data to Shaders : https://docs.vulkan.org/guide/latest/mapping_data_to_shaders.html
    //  - Descriptor Arrays : https://docs.vulkan.org/guide/latest/descriptor_arrays.html
    //  - Push Constants : https://docs.vulkan.org/guide/latest/push_constants.html
    // 
    /// @brief : A Pipeline Layout defines the resources that can be bound across the different shaders in a pipeline.
    ///  This comes in the form of Descriptor Sets and Push Constants.
    ///  - A Descriptor Set specifies the actual buffer or image resources that will be bound to the Shader at a given set index.
    ///  - A Descriptor Binding is one or more resources at a specific binding index in the Shader.
    ///  - A Push Constant is a small, single block of data that can have values set to it without the need of descriptors.
    //
    //  Example:
    //  Descriptor Set (0)                  // "SetIndex = 0". A Descriptor Set index in the Pipeline Layout, provided as an argument or bound to the pipeline.
    //      * DescriptorBinding (0)         // "BindingIndex = 0". GLSL: "layout(set = 0, binding = 0)". This is a fixed array of Descriptors.   
    //          - Descriptor (0)            // - Descriptor value at index 0 in the array.
    //          - Descriptor (1)            // - Descriptor value at index 1 in the array.
    //      * DescriptorBinding (1)         // "BindingIndex = 1". GLSL: "layout(set = 0, binding = 1)"
    //          - Descriptor (0)            // - Descriptor value.
    //
    //  Descriptor Set (1)
    //      * DescriptorBinding (0)         // GLSL: "layout(set = 1, binding = 0)"
    //          - Descriptor (0)
    //
    //  Push Constant Block
    //      * Offset (0), Size (16)         // 16 bytes of the block can be used to push data to.
    //----------------------------------------------------------------------------------------------------
    class PipelineLayout
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Binding information about a push constant. 
        //----------------------------------------------------------------------------------------------------
        struct PushConstantBindingDesc
        {
            vk::ShaderStageFlags m_stages {};
            uint32               m_offset = 0;
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Information about all resource bindings for a pipeline layout.  
        //----------------------------------------------------------------------------------------------------
        struct BindingInfo
        {
            std::vector<bool>                       m_hasVariableDescriptorCounts;         
            std::vector<DescriptorBindingDesc>      m_bindingDescs{};
            std::vector<DescriptorSetDesc>          m_setDescs{};
            std::vector<PushConstantBindingDesc>    m_pushConstantBindings{};
        };
        
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
        void                            SetDebugName(const std::string& name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the binding point for this pipeline. Can be graphics, compute, or ray tracing.
        //----------------------------------------------------------------------------------------------------
        vk::PipelineBindPoint           GetBindPoint() const            { return m_bindPoint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get information about each descriptor binding, each descriptor set and push constant bindings
        ///     for the pipeline.
        //----------------------------------------------------------------------------------------------------
        const BindingInfo&              GetBindingInfo() const          { return m_bindingInfo; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan Pipeline Layout resource.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::PipelineLayout& GetVkPipelineLayout() const     { return m_layout; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan Descriptor Set Layout for a given Descriptor Set in the pipeline.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::DescriptorSetLayout& GetVkDescriptorSetLayout(const uint32 setIndex) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject                  GetNativeVkObject() const;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the pipeline layout. 
        //----------------------------------------------------------------------------------------------------
        void                            CreatePipelineLayout(const RenderDevice& device, const PipelineLayoutDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the resource to the Renderer to be freed.
        //----------------------------------------------------------------------------------------------------
        void                            FreeLayout();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the descriptor set layout based on the description and adds it to the m_descriptorSetLayouts array. 
        //----------------------------------------------------------------------------------------------------
        void                            CreateSetLayout(const DescriptorSetDesc& setDesc);

    private:
        using DescriptorSetLayouts = std::vector<vk::raii::DescriptorSetLayout>;

        BindingInfo                     m_bindingInfo;
        RenderDevice*                   m_pDevice = nullptr;
        vk::raii::PipelineLayout        m_layout = nullptr;
        DescriptorSetLayouts            m_descriptorSetLayouts{};
        vk::PipelineBindPoint           m_bindPoint = vk::PipelineBindPoint::eGraphics;
    };

    static_assert(DeviceObjectType<PipelineLayout>);
}
