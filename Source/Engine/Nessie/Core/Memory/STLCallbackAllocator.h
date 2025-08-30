// STLCallbackAllocator.h
#pragma once
#include "AllocationCallbacks.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    template <typename Type>
    concept TypeAllowsSTLReallocate = std::is_trivially_copyable_v<Type> && TypeNeedsAlignedAllocate<Type>;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : STL-compatible allocator that is constructed with custom AllocationCallbacks.
    ///	@tparam Type : Type that will be allocated.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class STLCallbackAllocator
    {
    public:
        using value_type = Type;
        using pointer = Type*;
        using const_pointer = const Type*;
        using reference = Type&;
        using const_reference = const Type&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using is_always_equal = std::false_type;                        // This contains an allocation callback object.
        using propagate_on_container_move_assigment = std::true_type;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Default Ctor will get the default callbacks based on if the Type needs to be aligned or not. 
        //----------------------------------------------------------------------------------------------------
        STLCallbackAllocator()
        {
            m_callbacks = GetDefaultCallbacksForType<Type>();
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor to set custom allocation callbacks. If invalid, it will default to aligned allocations.
        //----------------------------------------------------------------------------------------------------
        STLCallbackAllocator(const AllocationCallbacks& callbacks)
        {
            m_callbacks = callbacks;
            m_callbacks.EnsureValidCallbacksOrReset();
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copy constructor.
        //----------------------------------------------------------------------------------------------------
        STLCallbackAllocator(const STLCallbackAllocator& other)
            : m_callbacks(other.GetCallbacks())
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copy constructor of another type.
        //----------------------------------------------------------------------------------------------------
        template <typename OtherType>
        STLCallbackAllocator(const STLCallbackAllocator<OtherType>& other)
            : m_callbacks(other.GetCallbacks())
        {
            //
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copy Assignment.
        //----------------------------------------------------------------------------------------------------
        STLCallbackAllocator& operator=(const STLCallbackAllocator& other)
        {
            if (this != &other)
                m_callbacks = other.GetCallbacks();
            
            return *this;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Boolean comparison operators.
        //----------------------------------------------------------------------------------------------------
        inline bool operator==(const STLCallbackAllocator& other) const { return m_callbacks == other.GetCallbacks(); }
        inline bool operator!=(const STLCallbackAllocator& other) const { return m_callbacks != other.GetCallbacks(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate memory.
        ///	@param count : Number of elements to allocate.
        //----------------------------------------------------------------------------------------------------
        pointer allocate(const size_type count)
        {
            return static_cast<pointer>(m_callbacks.Allocate(count * sizeof(Type), alignof(Type)));
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free memory.
        //----------------------------------------------------------------------------------------------------
        void deallocate(void* pPointer, size_type) const noexcept
        {
            m_callbacks.Free(pPointer);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reallocate Memory. Only available if the type is trivially copyable and doesn't need to
        ///     be aligned.
        //----------------------------------------------------------------------------------------------------
        inline pointer reallocate(pointer pOldPointer, const size_type /*oldSize*/, const size_t newSize) requires (TypeAllowsSTLReallocate<Type>)
        {
            NES_ASSERT(newSize > 0); // Reallocating to size zero is implementation-dependent, so we don't allow it.
            return pointer(m_callbacks.Reallocate(pOldPointer, newSize * sizeof(value_type), alignof(Type)));
        }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the callbacks that are used for this allocator. 
        //----------------------------------------------------------------------------------------------------
        const AllocationCallbacks& GetCallbacks() const { return m_callbacks; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : STL struct used to convert to another type.
        //----------------------------------------------------------------------------------------------------
        template <typename Type2>
        struct rebind
        {
            using other = STLCallbackAllocator<Type2>;
        };
    
    private:
        AllocationCallbacks m_callbacks{};
    };

    template <typename Type>
    concept IsValidSTLAllocator = requires(const Type& type)
    {
        std::vector<int, Type>(type);
    };
    static_assert(IsValidSTLAllocator<STLCallbackAllocator<int>>);
    
}
