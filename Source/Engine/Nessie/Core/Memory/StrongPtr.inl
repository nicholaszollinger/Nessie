// StrongPtr.inl
#pragma once

namespace nes
{
    template <typename Type>
    StrongPtr<Type>::StrongPtr(const StrongPtr& other)
        : m_pRefCounter(other.m_pRefCounter)
    {
        AddRef();
    }

    template <typename Type>
    StrongPtr<Type>::StrongPtr(StrongPtr&& other) noexcept
        : m_pRefCounter(other.m_pRefCounter)
    {
        other.m_pRefCounter = nullptr;
    }

    template <typename Type>
    StrongPtr<Type>::StrongPtr(Type* ptr)// requires nes::TypeIsDerivedFrom<Type, RefTarget<Type>>
    {
        static_assert(IsRefTargetType<Type>, "Only Types that inherit from RefTarget can assign a raw pointer to a StrongPtr, because they "
                                                                     "manage their ref count on the object itself.");
        m_pRefCounter = ptr;
        AddRef();
    }

    template <typename Type>
    StrongPtr<Type>& StrongPtr<Type>::operator=(nullptr_t)
    {
        RemoveRef();
        m_pRefCounter = nullptr;
        return *this;
    }

    template <typename Type>
    StrongPtr<Type>& StrongPtr<Type>::operator=(const StrongPtr& other)
    {
        if (this != &other)
        {
            RemoveRef();
            m_pRefCounter = other.m_pRefCounter;
            AddRef();
        }

        return *this;
    }

    template <typename Type>
    StrongPtr<Type>& StrongPtr<Type>::operator=(StrongPtr&& other) noexcept
    {
        if (this != &other)
        {
            RemoveRef();
            m_pRefCounter = other.m_pRefCounter;
            other.m_pRefCounter = nullptr;
        }

        return *this;
    }

    template <typename Type>
    template <typename OtherType>
    StrongPtr<Type>::StrongPtr(const StrongPtr<OtherType>& other)
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between StrongPtr Types!");
        
        if constexpr (IsRefTargetType<OtherType>)
        {
            auto* pCastedType = checked_cast<Type*>(other.m_pRefCounter);
            
            m_pRefCounter = pCastedType;
            pCastedType->AddRef();
        }

        else
        {
            internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<internal::RefCounter<OtherType>*>(other.m_pRefCounter);
            internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
            m_pRefCounter = pCastedCounter;
            pCastedCounter->AddRef();
        }
    }

    template <typename Type>
    template <typename OtherType>
    StrongPtr<Type>::StrongPtr(StrongPtr<OtherType>&& other) noexcept
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between StrongPtr Types!");
        
        if constexpr (IsRefTargetType<OtherType>)
        {
            auto* pCastedType = checked_cast<Type*>(other.m_pRefCounter);
            
            m_pRefCounter = pCastedType;
            other = nullptr;
        }

        else
        {
            internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<internal::RefCounter<OtherType>*>(other.m_pRefCounter);
            internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
            m_pRefCounter = pCastedCounter;
            other = nullptr;
        }
    }

    template <typename Type>
    template <typename OtherType>
    StrongPtr<Type>& StrongPtr<Type>::operator=(const StrongPtr<OtherType>& other)
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between StrongPtr Types!");
        
        if (this != &other)
        {
            if constexpr (IsRefTargetType<OtherType>)
            {
                auto* pCastedType = checked_cast<Type*>(other.m_pRefCounter);
            
                m_pRefCounter = pCastedType;
                pCastedType->AddRef();
            }

            else
            {
                internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<internal::RefCounter<OtherType>*>(other.m_pRefCounter);
                internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
                m_pRefCounter = pCastedCounter;
                pCastedCounter->AddRef();
            }
        }
        
        return *this;
    }

    template <typename Type>
    template <typename OtherType>
    StrongPtr<Type>& StrongPtr<Type>::operator=(StrongPtr<OtherType>&& other) noexcept
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between StrongPtr Types!");
        
        if (this != &other)
        {
            if constexpr (IsRefTargetType<OtherType>)
            {
                auto* pCastedType = checked_cast<Type*>(other.m_pRefCounter);
            
                m_pRefCounter = pCastedType;
                other = nullptr;
            }

            else
            {
                internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<internal::RefCounter<OtherType>*>(other.m_pRefCounter);
                internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
                m_pRefCounter = pCastedCounter;
                other = nullptr;
            }
        }
        
        return *this;
    }

    template <typename Type>
    Type* StrongPtr<Type>::Get() const
    {
        if (m_pRefCounter != nullptr)
            return static_cast<Type*>(m_pRefCounter->GetObject());

        return nullptr;
    }

    template <typename Type>
    void StrongPtr<Type>::Reset()
    {
        RemoveRef();
        m_pRefCounter = nullptr;
    }

    template <typename Type>
    void StrongPtr<Type>::AddRef() const
    {
        if (m_pRefCounter != nullptr)
            m_pRefCounter->AddRef();
        
    }

    template <typename Type>
    void StrongPtr<Type>::RemoveRef()
    {
        if (m_pRefCounter != nullptr && m_pRefCounter->RemoveRef())
        {
            if constexpr (!IsRefTargetType<Type>)
            {
                // We only call delete in the case of an external RefCounter object.
                // Removing the last reference of a RefTarget will delete the object
                // itself already.
                NES_DELETE(m_pRefCounter);
            }
            
            m_pRefCounter = nullptr;
        }
    }

    template <typename Type>
    ConstStrongPtr<Type>::ConstStrongPtr(const ConstStrongPtr& other)
        : m_pRefCounter(other.m_pRefCounter)
    {
        AddRef();
    }

    template <typename Type>
    ConstStrongPtr<Type>::ConstStrongPtr(ConstStrongPtr&& other) noexcept
        : m_pRefCounter(other.m_pRefCounter)
    {
        other.m_pRefCounter = nullptr;
    }

    template <typename Type>
    ConstStrongPtr<Type>::ConstStrongPtr(const StrongPtr<Type>& other)
        : m_pRefCounter(other.m_pRefCounter)
    {
        AddRef();
    }

    template <typename Type>
    ConstStrongPtr<Type>::ConstStrongPtr(StrongPtr<Type>&& other) noexcept
        : m_pRefCounter(other.m_pRefCounter)
    {
        other.m_pRefCounter = nullptr;
    }

    template <typename Type>
    ConstStrongPtr<Type>::ConstStrongPtr(const Type* pObject)
    {
        static_assert(IsRefTargetType<Type>, "Only Types that inherit from RefTarget can assign a raw pointer to a ConstStrongPtr, because they"
                                                                     "manage their ref count on the object itself.");
        m_pRefCounter = pObject;
        AddRef();
    }

    template <typename Type>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(nullptr_t)
    {
        RemoveRef();
        m_pRefCounter = nullptr;
        return *this;
    }

    template <typename Type>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(const ConstStrongPtr& other)
    {
        if (this != &other)
        {
            RemoveRef();
            m_pRefCounter = other.m_pRefCounter;
            AddRef();
        }

        return *this;
    }

    template <typename Type>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(ConstStrongPtr&& other) noexcept
    {
        if (this != &other)
        {
            RemoveRef();
            m_pRefCounter = other.m_pRefCounter;
            other.m_pRefCounter = nullptr;
        }

        return *this;
    }

    template <typename Type>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(const StrongPtr<Type>& other)
    {
        RemoveRef();
        m_pRefCounter = other.m_pRefCounter;
        AddRef();
        return *this;
    }

    template <typename Type>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(StrongPtr<Type>&& other) noexcept
    {
        RemoveRef();
        m_pRefCounter = other.m_pRefCounter;
        other.m_pRefCounter = nullptr;
        return *this;
    }

    template <typename Type>
    template <typename OtherType>
    ConstStrongPtr<Type>::ConstStrongPtr(const ConstStrongPtr<OtherType>& other)
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between ConstStrongPtr Types!");
        
        if constexpr (IsRefTargetType<OtherType>)
        {
            auto* pCastedType = checked_cast<const Type*>(other.m_pRefCounter);
            
            m_pRefCounter = pCastedType;
            m_pRefCounter->AddRef();
        }

        else
        {
            const internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<const internal::RefCounter<OtherType>*>(other.m_pRefCounter);
            const internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
            m_pRefCounter = pCastedCounter;
            m_pRefCounter->AddRef();
        }
    }

    template <typename Type>
    template <typename OtherType>
    ConstStrongPtr<Type>::ConstStrongPtr(ConstStrongPtr<OtherType>&& other) noexcept
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between ConstStrongPtr Types!");
        
        if constexpr (IsRefTargetType<OtherType>)
        {
            auto* pCastedType = checked_cast<const Type*>(other.m_pRefCounter);
            
            m_pRefCounter = pCastedType;
            other = nullptr;
        }

        else
        {
            const internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<const internal::RefCounter<OtherType>*>(other.m_pRefCounter);
            const internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
            m_pRefCounter = pCastedCounter;
            other = nullptr;
        }
    }

    template <typename Type>
    template <typename OtherType>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(const ConstStrongPtr<OtherType>& other)
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between ConstStrongPtr Types!");
        
        if (this != &other)
        {
            if constexpr (IsRefTargetType<OtherType>)
            {
                auto* pCastedType = checked_cast<const Type*>(other.m_pRefCounter);
                m_pRefCounter = pCastedType;
                m_pRefCounter->AddRef();
            }

            else
            {
                const internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<const internal::RefCounter<OtherType>*>(other.m_pRefCounter);
                const internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
                m_pRefCounter = pCastedCounter;
                m_pRefCounter->AddRef();
            }
        }

        return *this;
    }

    template <typename Type>
    template <typename OtherType>
    ConstStrongPtr<Type>& ConstStrongPtr<Type>::operator=(ConstStrongPtr<OtherType>&& other) noexcept
    {
        static_assert(TypeIsBaseOrDerived<Type, OtherType>, "Cannot convert between ConstStrongPtr Types!");
        
        if (this != &other)
        {
            if constexpr (IsRefTargetType<OtherType>)
            {
                auto* pCastedType = checked_cast<const Type*>(other.m_pRefCounter);
                m_pRefCounter = pCastedType;
                other = nullptr;
            }

            else
            {
                const internal::RefCounter<OtherType>* pFromRefCounter = reinterpret_cast<const internal::RefCounter<OtherType>*>(other.m_pRefCounter);
                const internal::RefCounter<Type>* pCastedCounter = pFromRefCounter->template GetAs<Type>();
            
                m_pRefCounter = pCastedCounter;
                other = nullptr;
            }
        }

        return *this;
    }

    template <typename Type>
    const Type* ConstStrongPtr<Type>::Get() const
    {
        if (m_pRefCounter != nullptr)
            return static_cast<const Type*>(m_pRefCounter->GetObject());

        return nullptr;
    }

    template <typename Type>
    void ConstStrongPtr<Type>::Reset()
    {
        RemoveRef();
        m_pRefCounter = nullptr;
    }

    template <typename Type>
    void ConstStrongPtr<Type>::AddRef() const
    {
        if (m_pRefCounter != nullptr)
            m_pRefCounter->AddRef();
    }

    template <typename Type>
    void ConstStrongPtr<Type>::RemoveRef()
    {
        if (m_pRefCounter != nullptr && m_pRefCounter->RemoveRef())
        {
            if constexpr (!IsRefTargetType<Type>)
            {
                // We only delete in the case of an external RefCounter object.
                // Removing the last reference of a RefTarget will delete the object
                // itself already.
                NES_DELETE(m_pRefCounter);
            }

            m_pRefCounter = nullptr;
        }
    }

    template <typename ObjectType, typename ... CtorParams> requires nes::ValidConstructorForType<ObjectType, CtorParams...>
    StrongPtr<ObjectType> Create(CtorParams&&...params)
    {
        ObjectType* pObject = NES_NEW(ObjectType(std::forward<CtorParams>(params)...)); 
        StrongPtr<ObjectType> result;
        if constexpr (IsRefTargetType<ObjectType>)
        {
            // The RefCounter is the actual object.
            result.m_pRefCounter = std::move(pObject);
        }
        
        else
        {
            // Create the external RefCounter and give it the object pointer.
            if (pObject != nullptr)
                result.m_pRefCounter = NES_NEW(internal::RefCounter<ObjectType>(pObject));
        }

        result.AddRef();
        
        return result;
    }

    template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
    StrongPtr<To> Cast(const StrongPtr<From>& ptr)
    {
        if (!ptr)
            return nullptr;

        if constexpr (IsRefTargetType<From>)
        {
            auto* pCastedType = dynamic_cast<To*>(ptr.m_pRefCounter);
            if (pCastedType == nullptr)
                return nullptr;
            
            StrongPtr<To> result;
            result.m_pRefCounter = pCastedType;
            pCastedType->AddRef();
            return result;
        }

        else
        {
            using FromRefCounterType = internal::RefCounter<From>;
            using ToRefCounterType = internal::RefCounter<To>;
            
            // reinterpret_cast from internal::RefCounterBase* -> internal::RefCounter<From*>
            FromRefCounterType* pRefCounter = reinterpret_cast<FromRefCounterType*>(ptr.m_pRefCounter);

            // Then, try to cast to the correct RefCounter type.
            ToRefCounterType* pCastedCounter = pRefCounter->template TryGetAs<To>();
            if (pCastedCounter == nullptr)
                return nullptr;

            StrongPtr<To> result;
            result.m_pRefCounter = pCastedCounter;
            pCastedCounter->AddRef();
            return result;
        }
    }

    template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
    ConstStrongPtr<To> Cast(const ConstStrongPtr<From>& ptr)
    {
        if (!ptr)
            return nullptr;

        if constexpr (IsRefTargetType<From>)
        {
            const auto* pCastedType = dynamic_cast<const To*>(ptr.m_pRefCounter);
            if (pCastedType == nullptr)
                return nullptr;
            
            ConstStrongPtr<To> result;
            result.m_pRefCounter = pCastedType;
            pCastedType->AddRef();
            return result;
        }

        else
        {
            using FromRefCounterType = internal::RefCounter<From>;
            using ToRefCounterType = internal::RefCounter<To>;
            
            // reinterpret_cast from internal::RefCounterBase* -> internal::RefCounter<From*>
            FromRefCounterType* pRefCounter = reinterpret_cast<FromRefCounterType*>(ptr.m_pRefCounter);

            // Then, try to cast to the correct RefCounter type.
            ToRefCounterType* pCastedCounter = pRefCounter->template TryGetAs<To>();
            if (pCastedCounter == nullptr)
                return nullptr;

            ConstStrongPtr<To> result;
            result.m_pRefCounter = pCastedCounter;
            pCastedCounter->AddRef();
            return result;
        }
    }
}
