// RefCounter.h
#pragma once
#include <cstdint>
#include "Memory.h"
#include "Debug/CheckedCast.h"

namespace nes::internal
{
    class RefCounterBase
    {
        /// A Large value that is added to the ref count, so that it remains in memory when the number of
        /// external references becomes zero.
        static constexpr uint32_t kEmbedded = 0x0ebedded;
        
    public:
        virtual ~RefCounterBase();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the underlying object. 
        //----------------------------------------------------------------------------------------------------
        virtual void*       GetObject() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the underlying object. 
        //----------------------------------------------------------------------------------------------------
        virtual const void* GetObject() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a "strong reference" to the underlying object. 
        //----------------------------------------------------------------------------------------------------
        void                AddRef() const              { m_refCount.fetch_add(1, std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a "strong reference" to the underlying object. If the ref count reaches zero, the
        ///     object will be deleted. However, if the object is embedded, it is up to the creator to properly
        ///     destroy the object.
        //----------------------------------------------------------------------------------------------------
        bool                RemoveRef() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current number of references to the object. 
        //----------------------------------------------------------------------------------------------------
        uint32_t            GetRefCount() const        { return m_refCount.load(std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Setting an object as embedded means to keep the object in memory even if the ref count
        ///     reaches zero.
        /// @note : If you set an object as embedded, it is on you to properly delete the resource!!!
        //----------------------------------------------------------------------------------------------------
        void                SetEmbedded() const         { m_refCount.fetch_add(kEmbedded, std::memory_order_relaxed); }
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Called when the ref count reaches zero. The underlying object must be destroyed.
        //----------------------------------------------------------------------------------------------------
        virtual void        ReleaseObject() const = 0;

    private:
        /// The current ref count to the underlying object. When this reaches zero, the object will be destroyed.
        mutable std::atomic<uint32> m_refCount = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A RefCounter manages ref count of an object externally.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class RefCounter final : public RefCounterBase
    {
    private:
        /// Private Ctor.
        RefCounter() = default;
    public:
        explicit            RefCounter(Type*&& pObject) : m_pObject(std::move(pObject)) { }

        template <typename To>
        RefCounter<To>*     GetAs();

    private:
        virtual void*       GetObject() override                { return m_pObject; }
        virtual const void* GetObject() const override          { return m_pObject; }
        virtual void        ReleaseObject() const override;

    private:
        mutable Type*       m_pObject = nullptr;
    };
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Inheriting from RefTarget places the refCount management on the actual object itself. The
    ///     disadvantage is that you are a small amount of data to the object, but the upside is that the
    ///     refCount remains stable when converting from a StrongPtr to a raw pointer.
    //----------------------------------------------------------------------------------------------------
    template <typename Derived>
    class RefTarget : public internal::RefCounterBase
    {
    public:
        using RefTargetDerivedType = Derived;

        RefTarget() = default;
        RefTarget(const RefTarget&)             { /* Do not copy over the ref count! */ }
        RefTarget& operator=(const RefTarget&)  { /* Do not copy over the ref count! */ return *this; }

    private:
        virtual void*       GetObject() override final                  { return this; }
        virtual const void* GetObject() const override final            { return this; }
        virtual void        ReleaseObject() const override final;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Release the Ref Target Object. By default, this calls delete on the pointer.
        ///	@param pThisObject : This is the 'this' pointer for this object! You can call delete in this function,
        ///     but make sure to not do anything else with this pointer if you do!
        //----------------------------------------------------------------------------------------------------
        virtual void        ReleaseObjectImpl(RefTargetDerivedType* pThisObject) const
        {
            NES_DELETE(pThisObject);
        }
    };

    template <typename Type>
    concept IsRefTargetType = requires(Type value)
    {
        TypeIsDerivedFrom<Type, internal::RefCounterBase>;
        TypeIsDerivedFrom<typename Type::RefTargetDerivedType, internal::RefCounterBase>;
    };
}

namespace nes::internal
{
    
//     //----------------------------------------------------------------------------------------------------
//     //		NOTES:
//     //		
//     ///		@brief : A RefCounter manages a pointer to an object and the reference count of that object.
//     ///     @note : This class is not thread-safe.
//     ///     @note : The Destructor will delete the object that it manages.
//     ///		@tparam Type : Type of Object that is being managed.
//     //----------------------------------------------------------------------------------------------------
//     template <typename Type>
//     class RefCounter
//     {
//         Type* m_pObject = nullptr;
//         uint32_t m_refCount = 0;
//         uint32_t m_weakCount = 0;
//
//     public:
//         explicit RefCounter(Type* ptr);
//
//         template <typename...Params> requires ValidConstructorForType<Type, Params...>
//         explicit RefCounter(Params&&...params);
//         ~RefCounter();
//
//         explicit operator bool() const { return m_pObject != nullptr; }
//         explicit operator bool()       { return m_pObject != nullptr; }
//
//         void AddRef()        { ++m_refCount; }
//         void RemoveRef();
//         void AddWeak()       { ++m_weakCount; }
//         void RemoveWeak()    { --m_weakCount; }
//
//         template<typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
//         RefCounter<OtherType>& Cast();
//
//         template<typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
//         const RefCounter<OtherType>& Cast() const;
//
//         [[nodiscard]] Type* Get() const                 { return m_pObject; }
//         [[nodiscard]] uint32_t GetRefCount() const      { return m_refCount; }
//         [[nodiscard]] uint32_t GetWeakCount() const     { return m_weakCount; }
//         [[nodiscard]] bool HasZeroReferences() const    { return m_refCount == 0 && m_weakCount == 0; }
//
//     private:
//         void DestroyObject();
//     };
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Construct a new RefCounter object. The object pointer is expected to not be null.
//     ///		@param ptr : Ptr to the new object.
//     //----------------------------------------------------------------------------------------------------
//     template <typename Type>
//     RefCounter<Type>::RefCounter(Type* ptr)
//         : m_pObject(ptr)
//     {
//         NES_ASSERT(m_pObject != nullptr);
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     //		NOTES:
//     //		
//     ///		@brief : Constructor for the RefCounter object that takes in parameters to construct the managed
//     ///             object.
//     ///		@tparam Params : Parameter Types required to construct the internal object of type "Type".
//     ///		@param params : Parameter values to construct the internal object.
//     //----------------------------------------------------------------------------------------------------
//     template <typename Type>
//     template <typename... Params> requires ValidConstructorForType<Type, Params...>
//     RefCounter<Type>::RefCounter(Params&&... params)
//     {
//         m_pObject = NES_NEW(Type(std::forward<Params>(params)...));
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Deletes the managed object.
//     //----------------------------------------------------------------------------------------------------
//     template <typename Type>
//     RefCounter<Type>::~RefCounter()
//     {
//         // Ensure that the Object and references were handled correctly.
//         NES_ASSERT(m_refCount == 0);
//         NES_ASSERT(m_weakCount == 0);
//         NES_ASSERT(m_pObject == nullptr);
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Remove a reference to the Object. If the reference count is 0, the object will be deleted.
//     //----------------------------------------------------------------------------------------------------
//     template <typename Type>
//     void RefCounter<Type>::RemoveRef()
//     {
//         // If the Ref count is 0, delete object.
//         --m_refCount;
//         if (m_refCount == 0)
//         {
//             DestroyObject();
//         }
//     }
//     
//     template <typename Type>
//     void RefCounter<Type>::DestroyObject()
//     {
//         // Delete the object.
//         NES_DELETE(m_pObject);
//         m_pObject = nullptr;
//     }
//
//     template <typename Type>
//     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
//     RefCounter<OtherType>& RefCounter<Type>::Cast()
//     {
// #if NES_LOGGING_ENABLED
//         // Perform a dynamic cast to ensure that the object is of the correct type.
//         // This is the check to see if the cast is valid.
//         [[maybe_unused]] OtherType* pOther = checked_cast<OtherType*>(m_pObject);
// #endif
//         // Reinterpret the RefCounter as a RefCounter of the new type.
//         return *(reinterpret_cast<RefCounter<OtherType>*>(this));
//     }
//
//     template <typename Type>
//     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
//     const RefCounter<OtherType>& RefCounter<Type>::Cast() const
//     {
// #if NES_LOGGING_ENABLED
//         // Perform a dynamic cast to ensure that the object is of the correct type.
//         // This is the check to see if the cast is valid.
//         [[maybe_unused]] OtherType* pOther = checked_cast<OtherType*>(m_pObject);
// #endif
//
//         // Reinterpret the RefCounter as a RefCounter of the new type.
//         return *(reinterpret_cast<const RefCounter<OtherType>*>(this));
//     }

}

#include "RefCounter.inl"