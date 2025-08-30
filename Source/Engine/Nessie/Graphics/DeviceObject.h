// GraphicsResource.h
#pragma once
#include "GraphicsCore.h"

namespace nes
{
    class RenderDevice;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains the raw pointer to the Vulkan Object, as well as the object type.
    //----------------------------------------------------------------------------------------------------
    struct NativeVkObject
    {
        void*           m_pHandle = nullptr;
        vk::ObjectType  m_type = vk::ObjectType::eUnknown;

        bool            operator==(const NativeVkObject& other) const { return m_type == other.m_type && m_pHandle == other.m_pHandle; }
        bool            operator!=(const NativeVkObject& other) const { return !(*this == other); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the type and handle are both valid.
        //----------------------------------------------------------------------------------------------------
        bool            IsValid() const { return m_pHandle != nullptr && m_type != vk::ObjectType::eUnknown; } 
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device Object contains references to one or more resources on the GPU.
    ///     They are designed as RAII classes ("Resource Acquisition is Initialization"), meaning that their
    ///     constructor performs initialization, and their destructor will release the GPU resource.
    ///     So when the Device Object leaves scope, its resource will be freed!
    ///
    ///     Device Objects are *not* copyable. They are only movable.
    ///
    ///     Device objects can be constructed with and assigned to nullptr. Constructing with nullptr is
    ///     effectively the default constructor. Assigning an initialized Device Object to nullptr will
    ///     free the resource.
    ///     
    /// <code>
    ///     nes::DeviceImage m_image = nullptr;             // Variables should be initialized to nullptr.
    ///     m_image = nes::DeviceImage(AllocateImageDesc&); // Constructs the image object.
    ///     m_image = nullptr;                              // Frees the image object.
    /// </code>
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept DeviceObjectType = requires(Type value)
    {
        Type(nullptr);                      // Constructor that takes in a std::nullptr_t value. Effectively destroys it.
        value = nullptr;                    // Can assign a std::nullptr_t value. Effectively destroys it.
        std::is_move_constructible_v<Type>; // Move Constructible.   
        std::is_move_assignable_v<Type>;    // Move Assignable.

        // Function that returns a Vulkan Object Type - it can be a C-type, Cpp type (vk namespace) or the vk::raii type.
        { value.GetNativeVkObject() } -> std::same_as<NativeVkObject>;  
    };

    //----------------------------------------------------------------------------------------------------
    /// Comparison operators for DeviceObject types.
    //----------------------------------------------------------------------------------------------------

    template <DeviceObjectType Type>
    bool operator==(const Type& a, const Type& b)
    {
        return a.GetNativeVkObject() == b.GetNativeVkObject();
    }

    template <DeviceObjectType Type>
    bool operator!=(const Type& a, const Type& b)
    {
        return a.GetNativeVkObject() != b.GetNativeVkObject();
    }

    template <DeviceObjectType Type>
    bool operator==(const Type& value, std::nullptr_t)
    {
        return value.GetNativeVkObject().m_pHandle == nullptr;
    }

    template <DeviceObjectType Type>
    bool operator==(std::nullptr_t, const Type& value)
    {
        return value.GetNativeVkObject().m_pHandle == nullptr;
    }

    template <DeviceObjectType Type>
    bool operator!=(const Type& value, std::nullptr_t)
    {
        return value.GetNativeVkObject().m_pHandle != nullptr;
    }

    template <DeviceObjectType Type>
    bool operator!=(std::nullptr_t, const Type& value)
    {
        return value.GetNativeVkObject().m_pHandle != nullptr;
    }
}
