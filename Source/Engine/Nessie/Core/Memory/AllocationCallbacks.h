// AllocationCallbacks.h
#pragma once
#include "Nessie/Core/Memory/Memory.h"

namespace nes
{
    template <typename Type>
    concept TypeNeedsAlignedAllocate = alignof(Type) > (NES_CPU_ADDRESS_BITS == 32? 8 : 16);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Set of functors for allocating memory (Allocate, Free, and Reallocate) and an optional
    ///     user pointer that will be used within the Allocation functions.
    ///
    ///     The callbacks will be defaulted to calls to their NES_ALIGNED counterparts, with the user pointer
    ///     being set to null.
    //----------------------------------------------------------------------------------------------------
    struct AllocationCallbacks
    {
        using AllocateFunction      = void* (*)(void* pUserData, size_t size, size_t alignment);
        using FreeFunction          = void  (*)(void* pUserData, void* pMemory);
        using ReallocationFunction  = void* (*)(void* pUserData, void* pOriginal, size_t size, size_t alignment);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default constructor will set defaults for all callbacks. The default is NES_ALIGNED_""()
        ///     with no user data.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor that sets all callbacks. You must provide all callbacks. If any are not provided, the
        ///     default implementation will be used.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks(const AllocateFunction& alloc, const FreeFunction& free, const ReallocationFunction& realloc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set all callbacks at once. You must provide all callbacks. If any are not provided, the
        ///     default implementation will be used.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks&    SetCallbacks(const AllocateFunction& alloc, const FreeFunction& free, const ReallocationFunction& realloc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user data to pass into the different callbacks. The default is nullptr.
        //----------------------------------------------------------------------------------------------------
        AllocationCallbacks&    SetUserData(void* pUserData);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate memory.
        //----------------------------------------------------------------------------------------------------
        void*                   Allocate(const size_t size, const size_t alignment) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free memory.
        //----------------------------------------------------------------------------------------------------
        void                    Free(void* pMemory) const;                                   

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reallocate memory.
        //----------------------------------------------------------------------------------------------------
        void*                   Reallocate(void* pOriginal, const size_t size, const size_t alignment) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ensure that *all* the callbacks are valid. If not, this will be reset to the default implementation. 
        //----------------------------------------------------------------------------------------------------
        void                    EnsureValidCallbacksOrReset();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get default callbacks for non-aligned allocation. The user pointer will be ignored.
        ///
        ///     Allocate = NES_ALLOC(), Free = NES_FREE(), Reallocate = NES_REALLOC().
        //----------------------------------------------------------------------------------------------------
        static AllocationCallbacks GetDefaultCallbacks();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get default callbacks that support aligned allocation. The user pointer will be ignored.
        ///
        ///     Allocate = NES_ALIGNED_ALLOC(), Free = NES_ALIGNED_FREE(), Reallocate = NES_ALIGNED_REALLOC().
        //----------------------------------------------------------------------------------------------------
        static AllocationCallbacks GetDefaultAlignedCallbacks();
    
        AllocateFunction        m_alloc;
        FreeFunction            m_free;
        ReallocationFunction    m_realloc;
        void*                   m_pUserData = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the default allocation callbacks based on whether the type should be aligned or not.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    AllocationCallbacks GetDefaultCallbacksForType()
    {
        if constexpr (TypeNeedsAlignedAllocate<Type>)
            return AllocationCallbacks::GetDefaultAlignedCallbacks();
        else
            return AllocationCallbacks::GetDefaultCallbacks();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allocate an object using custom allocation callbacks.
    //----------------------------------------------------------------------------------------------------
    template <typename Type, typename ...CtorParams>
    Type* Allocate(const AllocationCallbacks& callbacks, CtorParams&&...params)
    {
        Type* pObject = static_cast<Type*>(callbacks.Allocate(sizeof(Type), alignof(Type)));
        if (pObject)
        {
            new (pObject) Type(std::forward<CtorParams>(params)...);
        }
        return pObject;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Destroy an object using custom allocation callbacks. The pointer will be set to nullptr.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    void Free(const AllocationCallbacks& callbacks, Type*& pObject)
    {
        if (pObject)
        {
            pObject->~Type();           // Destruct
            callbacks.Free(pObject);    // Free
            pObject = nullptr;          // Set pointer to null.
        }
    }
}
