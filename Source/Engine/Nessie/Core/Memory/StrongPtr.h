#pragma once
#include "Nessie/Core/Memory/RefCounter.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Strong Pointer maintains ownership over the object it points to. When there are no
    ///     more references to the object, it will be deleted. If you wish to manually delete the object
    ///     later, then you can call SetEmbedded() which will keep the object in memory even if the
    ///     ref count hits zero, but it is on you to properly free the memory.
    ///	@tparam Type : Type of object that is pointed to.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class StrongPtr
    {
    public:
        /// Constructors & Destructor
        StrongPtr() = default;
        StrongPtr(std::nullptr_t) : StrongPtr() {}
        StrongPtr(const StrongPtr& other);
        StrongPtr(StrongPtr&& other) noexcept;
        StrongPtr(Type* ptr);
        ~StrongPtr()                                                                    { RemoveRef(); }
        
        /// Assignment Operators
        StrongPtr&                          operator=(nullptr_t);
        StrongPtr&                          operator=(const StrongPtr& other);
        StrongPtr&                          operator=(StrongPtr&& other) noexcept;

        /// Operators
        [[nodiscard]] inline                operator Type*() const                      { return Get(); }
        [[nodiscard]] inline                operator bool() const                       { return Get() != nullptr; }
        [[nodiscard]] inline Type*          operator->() const                          { return Get(); }
        [[nodiscard]] inline Type&          operator*() const                           { return *Get(); }
        [[nodiscard]] inline bool           operator==(const StrongPtr& other) const    { return m_pRefCounter == other.m_pRefCounter; }
        [[nodiscard]] inline bool           operator!=(const StrongPtr& other) const    { return !(*this == other); }
        [[nodiscard]] inline bool           operator==(const Type* pObject) const       { return Get() == pObject; }
        [[nodiscard]] inline bool           operator!=(const Type* pObject) const       { return Get() != pObject; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the raw object pointer. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] inline Type*          Get() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of references to this object.
        //----------------------------------------------------------------------------------------------------
        inline uint32_t                     GetRefCount() const                         { return m_pRefCounter->GetRefCount(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Embedding means that when the ref count reaches zero, the object will *not* be destroyed.
        /// @note : If you embed an object, it is up to you to properly delete that object!!!
        //----------------------------------------------------------------------------------------------------
        void                                SetEmbedded() const                          { m_pRefCounter->SetEmbedded(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resets this pointer to nullptr. Equivalent to <code>StrongPtr<Type> pPtr = nullptr</code> 
        //----------------------------------------------------------------------------------------------------
        void                                Reset();
        
    private:
        /// Friend classes:
        template <typename OtherType> friend class StrongPtr;
        template <typename OtherType> friend class ConstStrongPtr;

        /// Friend Functions:
        template <typename ObjectType, typename...CtorParams> requires nes::ValidConstructorForType<ObjectType, CtorParams...>
        friend StrongPtr<ObjectType>        Create(CtorParams&&...params);

        template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
        friend StrongPtr<To>                Cast(const StrongPtr<From>& ptr);
        
        void                                AddRef() const;
        void                                RemoveRef();

    private:
    internal::RefCounterBase*           m_pRefCounter = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Const Strong Pointer maintains ownership over a const reference to the object it points to.
    ///     When there are no more references to the object, it will be deleted. If you wish to manually delete the object
    ///     later, then you can call SetEmbedded() which will keep the object in memory even if the
    ///     ref count hits zero, but it is on you to properly free the memory.
    ///
    ///     StrongPtrs can be promoted to ConstStrongPtrs, but not the other way around.
    ///
    ///	@tparam Type : Type that this Pointer points to.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class ConstStrongPtr
    {
    public:
        /// Constructors
        ConstStrongPtr() = default;
        explicit ConstStrongPtr(nullptr_t) : ConstStrongPtr() {}
        ConstStrongPtr(const ConstStrongPtr& other);
        ConstStrongPtr(ConstStrongPtr&& other) noexcept;
        ConstStrongPtr(const StrongPtr<Type>& other);
        ConstStrongPtr(StrongPtr<Type>&& other) noexcept;
        ConstStrongPtr(const Type* pObject);
        ~ConstStrongPtr()                                                                   { RemoveRef(); }
        
        /// Assignment Operators
        ConstStrongPtr&                     operator=(nullptr_t);
        ConstStrongPtr&                     operator=(const ConstStrongPtr& other);
        ConstStrongPtr&                     operator=(ConstStrongPtr&& other) noexcept;
        ConstStrongPtr&                     operator=(const StrongPtr<Type>& other);
        ConstStrongPtr&                     operator=(StrongPtr<Type>&& other) noexcept;

        /// Operators
        [[nodiscard]] inline                operator const Type*() const                    { return Get(); }
        [[nodiscard]] inline                operator bool() const                           { return Get() != nullptr; }
        [[nodiscard]] inline const Type*    operator->() const                              { return Get(); }
        [[nodiscard]] inline const Type&    operator*() const                               { return *Get(); }
        [[nodiscard]] inline bool           operator==(const ConstStrongPtr& other) const   { return m_pRefCounter == other.m_pRefCounter; }
        [[nodiscard]] inline bool           operator!=(const ConstStrongPtr& other) const   { return !(*this == other); } 
        [[nodiscard]] inline bool           operator==(const StrongPtr<Type>& other) const  { return m_pRefCounter == other.m_pRefCounter; }
        [[nodiscard]] inline bool           operator!=(const StrongPtr<Type>& other) const  { return m_pRefCounter != other.m_pRefCounter; }
        [[nodiscard]] inline bool           operator==(const Type* pObject) const           { return Get() == pObject; }
        [[nodiscard]] inline bool           operator!=(const Type* pObject) const           { return Get() != pObject; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the raw object pointer. 
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] inline const Type*    Get() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of references to this object.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] inline uint32_t       GetRefCount() const                             { return m_pRefCounter->GetRefCount(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Embedding means that when the ref count reaches zero, the object will *not* be destroyed.
        /// @note : If you embed an object, it is up to you to properly delete that object!!!
        //----------------------------------------------------------------------------------------------------
        void                                SetEmbedded() const                             { m_pRefCounter->SetEmbedded(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resets this pointer to nullptr. Equivalent to <code>StrongPtr<Type> pPtr = nullptr</code> 
        //----------------------------------------------------------------------------------------------------
        void                                Reset();                                         

    private:
        /// Friend classes
        template <typename OtherType> friend class ConstStrongPtr;

        /// Friend functions
        template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
        friend ConstStrongPtr<To>           Cast(const ConstStrongPtr<From>& ptr);

        void                                AddRef() const;
        void                                RemoveRef();

    private:
        const internal::RefCounterBase*     m_pRefCounter = nullptr;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Dynamically allocates a new instance of ObjectType, and wraps it in a StrongPtr.  
    ///	@param params : Constructor params for the Object Type.
    //----------------------------------------------------------------------------------------------------
    template <typename ObjectType, typename...CtorParams> requires nes::ValidConstructorForType<ObjectType, CtorParams...>
    static StrongPtr<ObjectType>   Create(CtorParams&&...params);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Cast a StrongPtr<From> to StrongPtr<To>.
    //----------------------------------------------------------------------------------------------------
    template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
    static StrongPtr<To>           Cast(const StrongPtr<From>& ptr);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Cast a ConstStrongPtr<From> to ConstStrongPtr<To>.
    //----------------------------------------------------------------------------------------------------
    template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
    static ConstStrongPtr<To>      Cast(const ConstStrongPtr<From>& ptr);
}

#include "StrongPtr.inl"