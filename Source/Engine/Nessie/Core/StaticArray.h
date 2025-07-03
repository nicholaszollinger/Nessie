// StaticArray.h
#pragma once
#include <cstdint>
#include <initializer_list>
#include "Nessie/Core/Concepts.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Fixed-size-capacity, variable-length array.
    ///	@tparam ElementType : Element type of the array. 
    ///	@tparam N : Max Capacity of the array. 
    //----------------------------------------------------------------------------------------------------
    template <typename ElementType, size_t N>
    class StaticArray
    {
        static_assert((std::is_copy_constructible_v<ElementType> || std::is_trivially_copyable_v<ElementType>), "Type must be copy constructible.");
        static_assert((std::is_copy_assignable_v<ElementType>), "Type must be copy assignable.");
        //static_assert(std::equality_comparable<ElementType>, "Type must be equality comparable");
    
    public:
        using value_type        = ElementType;
        using size_type         = size_t;
        using const_iterator    = const ElementType*;
        using iterator          = ElementType*;

    protected:
        struct alignas(ElementType) Storage
        {
            uint8_t m_data[sizeof(ElementType)];
        };
    
        static constexpr size_t kCapacity = N;

    public:
        StaticArray() = default;
        explicit StaticArray(std::initializer_list<value_type> list);
        StaticArray(const StaticArray& other);
        ~StaticArray();
        
        StaticArray&           operator=(const StaticArray& other);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Assignment operator with a static vector of another size.
        /// @note : M must be less than or equal to the size of this vector!
        //----------------------------------------------------------------------------------------------------
        template <size_t M>
        StaticArray&           operator=(const StaticArray<ElementType, M>& other);      
        
        ElementType&            operator[](size_type index);
        const ElementType&      operator[](size_type index) const;
        bool                    operator==(const StaticArray& other) const;
        bool                    operator!=(const StaticArray& other) const { return !(*this == other); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access an element at a given index. 
        //----------------------------------------------------------------------------------------------------
        ElementType&            at(const size_type index);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access an element at a given index. 
        //----------------------------------------------------------------------------------------------------
        const ElementType&      at(const size_type index) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the first element in the array. 
        //----------------------------------------------------------------------------------------------------
        ElementType&            front();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the first element in the array. 
        //----------------------------------------------------------------------------------------------------
        const ElementType&      front() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the last element in the array. 
        //----------------------------------------------------------------------------------------------------
        ElementType&            back();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the last element in the array. 
        //----------------------------------------------------------------------------------------------------
        const ElementType&      back() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destructs all elements and set the size to 0. 
        //----------------------------------------------------------------------------------------------------
        void                    clear();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an element to the back of the array. 
        //----------------------------------------------------------------------------------------------------
        void                    push_back(const ElementType& value);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct an element at the back of the array.
        //----------------------------------------------------------------------------------------------------
        template <typename... CtorArgs> requires ValidConstructorForType<ElementType, CtorArgs...>
        void                    emplace_back(CtorArgs&&...params);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove last element in the array. 
        //----------------------------------------------------------------------------------------------------
        void                    pop_back();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if there are no elements in the array. 
        //----------------------------------------------------------------------------------------------------
        bool                    empty() const              { return m_size == 0; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the number of elements in the array. 
        //----------------------------------------------------------------------------------------------------
        size_type               size() const          { return m_size; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the maximum number of elements the array can hold.
        //----------------------------------------------------------------------------------------------------
        constexpr size_type     capacity() const       { return kCapacity; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize to a new length array. 
        //----------------------------------------------------------------------------------------------------
        void                    resize(size_type newSize);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a const iterator to the first element in the array.
        //----------------------------------------------------------------------------------------------------
        const_iterator          begin() const       { return reinterpret_cast<const ElementType*>(m_elements); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a const iterator to the end of the array. (1 past the last element). 
        //----------------------------------------------------------------------------------------------------
        const_iterator          end() const         { return reinterpret_cast<const ElementType*>(m_elements + m_size); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an iterator to the first element in the array.
        //----------------------------------------------------------------------------------------------------
        iterator                begin()             { return reinterpret_cast<ElementType*>(m_elements); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an iterator to the end of the array. (1 past the last element). 
        //----------------------------------------------------------------------------------------------------
        iterator                end()               { return reinterpret_cast<ElementType*>(m_elements + m_size); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a const raw pointer to the start of the array.
        //----------------------------------------------------------------------------------------------------
        const ElementType*      data() const        { return  reinterpret_cast<const ElementType*>(m_elements); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a raw pointer to the start of the array.
        //----------------------------------------------------------------------------------------------------
        ElementType*            data()              { return  reinterpret_cast<ElementType*>(m_elements); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove an element from the array. 
        //----------------------------------------------------------------------------------------------------
        void                    erase(const_iterator it);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove multiple elements from the array.
        ///	@param first : Position to begin removal.
        ///	@param last : Position to end removal.
        //----------------------------------------------------------------------------------------------------
        void                    erase(const_iterator first, const_iterator last);

    protected:
        Storage                 m_elements[kCapacity];  /// Fixed Buffer of elements.
        size_type               m_size = 0;             /// Count of constructed elements in the array.
    };

    template <typename ElementType, size_t N>
    StaticArray<ElementType, N>::StaticArray(std::initializer_list<value_type> list)
    {
        NES_ASSERT(list.size() <= N);
        for (const ElementType& value : list)
        {
            new (reinterpret_cast<ElementType*>(&m_elements[m_size++])) ElementType(value);
        }
    }

    template <typename ElementType, size_t N>
    StaticArray<ElementType, N>::StaticArray(const StaticArray& other)
    {
        while (m_size < other.m_size)
        {
            new (&m_elements[m_size]) ElementType(other[m_size]);
            ++m_size;
        }
    }

    template <typename ElementType, size_t N>
    StaticArray<ElementType, N>::~StaticArray()
    {
        if constexpr (!std::is_trivially_destructible_v<ElementType>)
        {
            for (ElementType* pCurrent = reinterpret_cast<ElementType*>(m_elements), *pEnd = pCurrent + m_size; pCurrent < pEnd; ++pCurrent) 
            {
                pCurrent->~ElementType();
            }
        }
    }

    template <typename ElementType, size_t N>
    StaticArray<ElementType, N>& StaticArray<ElementType, N>::operator=(const StaticArray& other)
    {
        const size_type otherSize = other.m_size;

        if (static_cast<const void*>(this) != static_cast<const void*>(&other))
        {
            clear();

            while (m_size < otherSize)
            {
                new (&m_elements[m_size]) ElementType(other[m_size]);
                ++m_size;
            }
        }

        return *this;
    }

    template <typename ElementType, size_t N>
    template <size_t M>
    StaticArray<ElementType, N>& StaticArray<ElementType, N>::operator=(const StaticArray<ElementType, M>& other)
    {
        const size_type otherSize = other.size();
        NES_ASSERT(otherSize <= kCapacity);

        if (static_cast<const void*>(this) != static_cast<const void*>(&other))
        {
            clear();

            while (m_size < otherSize)
            {
                new (&m_elements[m_size]) ElementType(other[m_size]);
                ++m_size;
            }
        }

        return *this;
    }

    template <typename ElementType, size_t N>
    ElementType& StaticArray<ElementType, N>::operator[](size_type index)
    {
        NES_ASSERT(index < m_size);
        return reinterpret_cast<ElementType&>(m_elements[index]);
    }

    template <typename ElementType, size_t N>
    const ElementType& StaticArray<ElementType, N>::operator[](size_type index) const
    {
        NES_ASSERT(index < m_size);
        return reinterpret_cast<const ElementType&>(m_elements[index]);
    }

    template <typename ElementType, size_t N>
    bool StaticArray<ElementType, N>::operator==(const StaticArray& other) const
    {
        if (m_size != other.m_size)
            return false;

        // Compare each element
        for (size_type i = 0; i < m_size; ++i)
        {
            if (!(reinterpret_cast<const ElementType&>(m_elements[i]) == reinterpret_cast<const ElementType&>(other.m_elements[i])))
                return false;
        }

        return true;
    }

    template <typename ElementType, size_t N>
    ElementType& StaticArray<ElementType, N>::at(const size_type index)
    {
        NES_ASSERT(index < m_size);
        return reinterpret_cast<ElementType&>(m_elements[index]);
    }

    template <typename ElementType, size_t N>
    const ElementType& StaticArray<ElementType, N>::at(const size_type index) const
    {
        NES_ASSERT(index < m_size);
        return reinterpret_cast<const ElementType&>(m_elements[index]);
    }

    template <typename ElementType, size_t N>
    ElementType& StaticArray<ElementType, N>::front()
    {
        NES_ASSERT(m_size > 0);
        return reinterpret_cast<ElementType&>(m_elements[0]);
    }

    template <typename ElementType, size_t N>
    const ElementType& StaticArray<ElementType, N>::front() const
    {
        NES_ASSERT(m_size > 0);
        return reinterpret_cast<const ElementType&>(m_elements[0]);
    }

    template <typename ElementType, size_t N>
    ElementType& StaticArray<ElementType, N>::back()
    {
        NES_ASSERT(m_size > 0);
        return reinterpret_cast<ElementType&>(m_elements[m_size - 1]);
    }

    template <typename ElementType, size_t N>
    const ElementType& StaticArray<ElementType, N>::back() const
    {
        NES_ASSERT(m_size > 0);
        return reinterpret_cast<const ElementType&>(m_elements[m_size - 1]);
    }

    template <typename ElementType, size_t N>
    void StaticArray<ElementType, N>::clear()
    {
        if constexpr (!std::is_trivially_destructible_v<ElementType>)
        {
            for (ElementType* pCurrent = reinterpret_cast<ElementType*>(m_elements), *pEnd = pCurrent + m_size; pCurrent < pEnd; ++pCurrent) 
            {
                pCurrent->~ElementType();
            }
        }

        m_size = 0;
    }

    template <typename ElementType, size_t N>
    void StaticArray<ElementType, N>::push_back(const ElementType& value)
    {
        NES_ASSERT(m_size < N);
        new (&m_elements[m_size++]) ElementType(value);
    }

    template <typename ElementType, size_t N>
    template <typename ... CtorArgs> requires ValidConstructorForType<ElementType, CtorArgs...>
    void StaticArray<ElementType, N>::emplace_back(CtorArgs&&... params)
    {
        NES_ASSERT(m_size < N);
        new (&m_elements[m_size++]) ElementType(std::forward<CtorArgs>(params)...);
    }

    template <typename ElementType, size_t N>
    void StaticArray<ElementType, N>::pop_back()
    {
        NES_ASSERT(m_size > 0);
        reinterpret_cast<ElementType&>(m_elements[--m_size]).~ElementType();
    }

    template <typename ElementType, size_t N>
    void StaticArray<ElementType, N>::resize(size_type newSize)
    {
        NES_ASSERT(newSize <= N);

        ElementType* pBegin = reinterpret_cast<ElementType*>(m_elements);

        // Construct (newSize - m_size) elements at the end of the array.
        if constexpr (std::is_trivially_constructible_v<ElementType>)
        {
            for (ElementType* pCurrent = pBegin + m_size, *pEnd = pBegin + newSize; pCurrent < pEnd; ++pCurrent)
            {
                new (pCurrent) ElementType;
            }
        }

        // Destruct elements if newSize less than the original size.   
        if constexpr (!std::is_trivially_destructible_v<ElementType>)
        {
            for (ElementType* pCurrent = pBegin + newSize, *pEnd = pBegin + m_size; pCurrent < pEnd; ++pCurrent)
            {
                pCurrent->~ElementType();
            }
        }

        m_size = newSize;
    }

    template <typename ElementType, size_t N>
    void StaticArray<ElementType, N>::erase(const_iterator it)
    {
        const size_type position = reinterpret_cast<size_type>(it - begin());
        NES_ASSERT(position < m_size);

        if constexpr (!std::is_trivially_destructible_v<ElementType>)
        {
            reinterpret_cast<ElementType&>(m_elements[position]).~ElementType();
        }

        if (position + 1 < m_size)
        {
            memmove(m_elements + position, m_elements + position + 1, (m_size - position - 1) * sizeof(ElementType));
        }

        --m_size;
    }

    template <typename ElementType, size_t N>
    void StaticArray<ElementType, N>::erase(const_iterator first, const_iterator last)
    {
        NES_ASSERT(last <= end());
        
        const size_type position = reinterpret_cast<size_type>(first - begin());
        const size_type count = reinterpret_cast<size_type>(last - first);

        if constexpr (!std::is_trivially_destructible_v<ElementType>)
        {
            for (size_type i = 0; i < count; ++i)
            {
                reinterpret_cast<ElementType&>(m_elements[position + i]).~ElementType();
            }
        }

        if (position + count < m_size)
        {
            memmove(m_elements + position, m_elements + position + count, (m_size - position - count) * sizeof(ElementType));
        }

        m_size -= count;
    }
}
