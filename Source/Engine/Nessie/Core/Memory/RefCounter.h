#pragma once
// RefCounter.h
#include <BleachNew.h>
#include <cstdint>
#include "Core/Debug/CheckedCast.h"

namespace nes::internal
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A RefCounter manages a pointer to an object and the reference count of that object.
    ///     @note : This class is not thread-safe.
    ///     @note : The Destructor will delete the object that it manages.
    ///		@tparam Type : Type of Object that is being managed.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class RefCounter
    {
        Type* m_pObject = nullptr;
        uint32_t m_refCount = 0;
        uint32_t m_weakCount = 0;

    public:
        explicit RefCounter(Type* ptr);

        template <typename...Params> requires ValidConstructorForType<Type, Params...>
        explicit RefCounter(Params&&...params);
        ~RefCounter();

        explicit operator bool() const { return m_pObject != nullptr; }
        explicit operator bool()       { return m_pObject != nullptr; }

        void AddRef()        { ++m_refCount; }
        void RemoveRef();
        void AddWeak()       { ++m_weakCount; }
        void RemoveWeak()    { --m_weakCount; }

        template<typename OtherType> requires nes::IsBaseOrDerived<Type, OtherType>
        RefCounter<OtherType>& Cast();

        template<typename OtherType> requires nes::IsBaseOrDerived<Type, OtherType>
        const RefCounter<OtherType>& Cast() const;

        [[nodiscard]] Type* Get() { return m_pObject; }
        [[nodiscard]] const Type* Get() const { return m_pObject; }
        [[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }
        [[nodiscard]] uint32_t GetWeakCount() const { return m_weakCount; }
        [[nodiscard]] bool HasZeroReferences() const { return m_refCount == 0 && m_weakCount == 0; }

    private:
        void DestroyObject();
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Construct a new RefCounter object. The object pointer is expected to not be null.
    ///		@param ptr : Ptr to the new object.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    RefCounter<Type>::RefCounter(Type* ptr)
        : m_pObject(ptr)
    {
        NES_ASSERT(m_pObject != nullptr);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Constructor for the RefCounter object that takes in parameters to construct the managed
    ///             object.
    ///		@tparam Params : Parameter Types required to construct the internal object of type "Type".
    ///		@param params : Parameter values to construct the internal object.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    template <typename... Params> requires ValidConstructorForType<Type, Params...>
    RefCounter<Type>::RefCounter(Params&&... params)
    {
        m_pObject = BLEACH_NEW(Type(std::forward<Params>(params)...));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Deletes the managed object.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    RefCounter<Type>::~RefCounter()
    {
        // Ensure that the Object and references were handled correctly.
        NES_ASSERT(m_refCount == 0);
        NES_ASSERT(m_weakCount == 0);
        MY_ASSERT(m_pObject == nullptr);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Remove a reference to the Object. If the reference count is 0, the object will be deleted.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    void RefCounter<Type>::RemoveRef()
    {
        // If the Ref count is 0, delete object.
        --m_refCount;
        if (m_refCount == 0)
        {
            DestroyObject();
        }
    }

    template <typename Type>
    void RefCounter<Type>::DestroyObject()
    {
        // Delete the object.
        BLEACH_DELETE(m_pObject);
        m_pObject = nullptr;
    }

    template <typename Type>
    template <typename OtherType> requires nes::IsBaseOrDerived<Type, OtherType>
    RefCounter<OtherType>& RefCounter<Type>::Cast()
    {
#if NES_LOGGING_ENABLED
        // Perform a dynamic cast to ensure that the object is of the correct type.
        // This is the check to see if the cast is valid.
        [[maybe_unused]] OtherType* pOther = checked_cast<OtherType*>(m_pObject);
#endif
        // Reinterpret the RefCounter as a RefCounter of the new type.
        return *(reinterpret_cast<RefCounter<OtherType>*>(this));
    }

    template <typename Type>
    template <typename OtherType> requires nes::IsBaseOrDerived<Type, OtherType>
    const RefCounter<OtherType>& RefCounter<Type>::Cast() const
    {
#if NES_LOGGING_ENABLED
        // Perform a dynamic cast to ensure that the object is of the correct type.
        // This is the check to see if the cast is valid.
        [[maybe_unused]] OtherType* pOther = checked_cast<OtherType*>(m_pObject);
#endif

        // Reinterpret the RefCounter as a RefCounter of the new type.
        return *(reinterpret_cast<const RefCounter<OtherType>*>(this));
    }

}