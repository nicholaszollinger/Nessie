// DescriptorPool.h
#pragma once
#include "DescriptorSet.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Descriptor Sets cannot be created directly, they must be allocated by a Descriptor Pool.
    ///     A Descriptor Pool is created with max number descriptors for each on their type.
    //----------------------------------------------------------------------------------------------------
    class DescriptorPool
    {
    public:
        DescriptorPool(std::nullptr_t) {}
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool(DescriptorPool&& other) noexcept;
        DescriptorPool& operator=(std::nullptr_t);
        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool& operator=(DescriptorPool&& other) noexcept;
        ~DescriptorPool();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates a new Descriptor Pool.
        //----------------------------------------------------------------------------------------------------
        DescriptorPool(RenderDevice& device, const DescriptorPoolDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate one or more descriptors based on a DescriptorSetLayout in the PipelineLayout.
        ///	@param layout : The PipelineLayout that owns the DescriptorSetLayout at the given setIndex.  
        ///	@param setIndex : The PipelineLayout owns DescriptorSetLayouts. This is the index into the PipelineLayout's array of DescriptorSetLayouts.
        ///	@param pOutSets : Array (or pointer to a single value) that will store the created DescriptorSets. 
        ///	@param numInstances : The number of Descriptor Sets that you want to create.
        ///	@param numVariableDescriptors : If the Descriptor is a variable sized array, this is number of elements in that array.
        //----------------------------------------------------------------------------------------------------
        void                        AllocateDescriptorSets(const PipelineLayout& layout, const uint32 setIndex, DescriptorSet* pOutSets, const uint32 numInstances = 1, const uint32 numVariableDescriptors = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Frees all DescriptorSets allocated from this pool. 
        //----------------------------------------------------------------------------------------------------
        void                        Reset();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Debug name for this Descriptor Pool. 
        //----------------------------------------------------------------------------------------------------
        void                        SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject              GetNativeVkObject() const;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the Descriptor Pool resource to the Renderer to be freed. 
        //----------------------------------------------------------------------------------------------------
        void                        FreePool();
        
    private:
        RenderDevice*               m_pDevice = nullptr;
        Mutex                       m_mutex;
        vk::raii::DescriptorPool    m_pool = nullptr;
    };

    static_assert(DeviceObjectType<DescriptorPool>);
}
