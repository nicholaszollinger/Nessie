// DescriptorSet.h
#pragma once
#include "DeviceObject.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Descriptor Set is the set of values for a group of bindings that can be bound to a Shader to use. 
    //----------------------------------------------------------------------------------------------------
    class DescriptorSet
    {
    public:
        DescriptorSet(std::nullptr_t) {}
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet(DescriptorSet&& other) noexcept;
        DescriptorSet& operator=(std::nullptr_t);
        DescriptorSet& operator=(const DescriptorSet&) = delete;
        DescriptorSet& operator=(DescriptorSet&& other) noexcept;
        ~DescriptorSet();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update a number of bindings with descriptor values.
        ///	@param pUpdateDescs : Array of update values for each binding that you want to update.
        ///	@param firstBinding : Index of the first binding in the set to update.
        ///	@param numBindings : Number of bindings that we are updating. This must be equal to the number of update descriptions. 
        //----------------------------------------------------------------------------------------------------
        void                        UpdateBindings(const DescriptorBindingUpdateDesc* pUpdateDescs, uint32 firstBinding, uint32 numBindings = 1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Debug name for this Descriptor Set. 
        //----------------------------------------------------------------------------------------------------
        void                        SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan Descriptor Set object.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::DescriptorSet& GetVkDescriptorSet() const { return m_set; }    

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject              GetNativeVkObject() const;

    private:
        friend class DescriptorPool;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Private constructor for the Descriptor Pool to use.
        //----------------------------------------------------------------------------------------------------
        DescriptorSet(RenderDevice& device, const DescriptorSetDesc* pDesc, vk::raii::DescriptorSet&& set);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submit the Descriptor Set to the Renderer to be freed. 
        //----------------------------------------------------------------------------------------------------
        void                        FreeSet();

    private:
        RenderDevice*               m_pDevice = nullptr;
        vk::raii::DescriptorSet     m_set = nullptr;
        const DescriptorSetDesc*    m_pDesc = nullptr;
    };

    static_assert(DeviceObjectType<DescriptorSet>);
}
