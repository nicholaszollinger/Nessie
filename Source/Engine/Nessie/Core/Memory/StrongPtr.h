#pragma once
#include "RefCounter.h"

namespace nes
{
    template <typename Type>
    class WeakPtr;

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
        internal::RefCounterBase* m_pRefCounter = nullptr;
    
    public:
        /// Constructors
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
        template <typename OtherType> friend class StrongPtr;
        template <typename OtherType> friend class ConstStrongPtr;

        // [TODO]: 
        // WeakPtr needs access to the RefCounter.
        template <typename OtherType> friend class WeakPtr; 
        
        template <typename ObjectType, typename...CtorParams> requires nes::ValidConstructorForType<ObjectType, CtorParams...>
        friend static StrongPtr<ObjectType> Create(CtorParams&&...params);

        template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
        friend static StrongPtr<To>         Cast(const StrongPtr<From>& ptr);
        
        void                                AddRef() const;
        void                                RemoveRef();
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
        const internal::RefCounterBase* m_pRefCounter = nullptr;

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
        template <typename OtherType> friend class ConstStrongPtr;
        
        template <typename To, typename From> requires nes::TypeIsBaseOrDerived<To, From>
        friend static ConstStrongPtr<To>    Cast(const ConstStrongPtr<From>& ptr);

        void                                AddRef() const;
        void                                RemoveRef();
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

    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //		
    // ///		brief : WeakPtr is a non-owning pointer to an object.
    // ///		@tparam Type : Type of Object that this WeakPtr points to.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // class WeakPtr
    // {
    //     // Give private access to StrongPtr types of derived classes.
    //     // This allows the casting to work correctly.
    //     template <typename OtherType>
    //     friend class WeakPtr;
    //
    //     internal::RefCounter<Type>* m_pRefCounter = nullptr;
    //
    //     // Internal constructor for the RefCounter.
    //     WeakPtr(internal::RefCounter<Type>* pRefCounter);
    //
    // public:
    //     WeakPtr() = default;
    //     WeakPtr(std::nullptr_t) : WeakPtr() {}
    //     WeakPtr(const StrongPtr<Type>& pStrongPtr);
    //     WeakPtr(const WeakPtr& other);
    //     WeakPtr(WeakPtr&& other) noexcept;
    //     WeakPtr& operator=(const WeakPtr& other);
    //     WeakPtr& operator=(WeakPtr&& other) noexcept;
    //     ~WeakPtr();
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr(const WeakPtr<OtherType>& other);
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr(const StrongPtr<OtherType>& other);
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr(StrongPtr<OtherType>&& other) noexcept;
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr& operator=(const WeakPtr<OtherType>& other);
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr& operator=(WeakPtr<OtherType>&& other) noexcept;
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr& operator=(const StrongPtr<OtherType>& other);
    //
    //     [[nodiscard]] StrongPtr<Type> Lock() const;
    //     [[nodiscard]] bool operator==(const WeakPtr& other) const { return m_pRefCounter == other.m_pRefCounter; }
    //     [[nodiscard]] bool operator!=(const WeakPtr& other) const { return !(*this == other); }
    //     [[nodiscard]] explicit operator bool() { return IsValid(); }
    //     [[nodiscard]] explicit operator bool() const { return IsValid(); }
    //     [[nodiscard]] bool IsValid() const;
    //
    //     template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    //     WeakPtr<OtherType> Cast() const;
    //
    // private:
    //     void AddRef();
    //     void RemoveRef();
    // };
}

namespace nes
{
    // template <typename Type>
    // StrongPtr<Type>::StrongPtr(internal::RefCounter<Type>* pRefCounter)
    //     : m_pRefCounter(pRefCounter)
    // {
    //     AddRef();
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Construct the StrongPtr with a raw pointer to the Type.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // StrongPtr<Type>::StrongPtr(Type* ptr)
    // {
    //     if (ptr != nullptr)
    //     {
    //         m_pRefCounter = NES_NEW(internal::RefCounter<Type>(ptr));
    //         AddRef();
    //     }
    // }
    //
    // template <typename Type>
    // StrongPtr<Type>::StrongPtr(const StrongPtr& other)
    // {
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = other.m_pRefCounter;
    //         AddRef();
    //     }
    // }
    //
    // template <typename Type>
    // StrongPtr<Type>::StrongPtr(StrongPtr&& other) noexcept
    // {
    //     // Steal the pointer and don't add a reference.
    //     m_pRefCounter = other.m_pRefCounter;
    //     other.m_pRefCounter = nullptr;
    // }
    //
    // template <typename Type>
    // StrongPtr<Type>& StrongPtr<Type>::operator=(const StrongPtr& other)
    // {
    //     if (this != &other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         if (other.m_pRefCounter != nullptr)
    //         {
    //             m_pRefCounter = other.m_pRefCounter;
    //             AddRef();
    //         }
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // StrongPtr<Type>& StrongPtr<Type>::operator=(StrongPtr&& other) noexcept
    // {
    //     if (this != &other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         m_pRefCounter = other.m_pRefCounter;
    //         other.m_pRefCounter = nullptr;
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // StrongPtr<Type>::~StrongPtr()
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         RemoveRef();
    //     }
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // StrongPtr<Type>::StrongPtr(const StrongPtr<OtherType>& other)
    // {
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //         AddRef();
    //     }
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // StrongPtr<Type>::StrongPtr(StrongPtr<OtherType>&& other)
    // {
    //     // Steal the pointer, don't add a reference.
    //     m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //     other.m_pRefCounter = nullptr;
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // StrongPtr<Type>& StrongPtr<Type>::operator=(const StrongPtr<OtherType>& other)
    // {
    //     if (this != &other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         if (other.m_pRefCounter != nullptr)
    //         {
    //             m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //             AddRef();
    //         }
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // StrongPtr<Type>& StrongPtr<Type>::operator=(StrongPtr<OtherType>&& other) noexcept
    // {
    //     if (this != &other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         // Steal the pointer, but don't add a reference.
    //         m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //         other.m_pRefCounter = nullptr;
    //     }
    //
    //     return *this;
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //		
    // ///		@brief : Reset this pointer to null. 
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // void StrongPtr<Type>::Reset()
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         RemoveRef();
    //     }
    //
    //     m_pRefCounter = nullptr;
    // }
    //
    // template <typename Type>
    // template <typename To> requires nes::TypeIsBaseOrDerived<Type, To>
    // StrongPtr<To> StrongPtr<Type>::Cast() const
    // {
    //     return StrongPtr<To>(*this);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //		
    // ///		@brief : Create a new StrongPtr, constructing a new object of Type.
    // ///     @tparam Type : Type of object to create.
    // ///		@tparam Params : Parameter Types to be passed to the Type constructor.
    // ///		@param params : Parameter values to be passed to the Type constructor.
    // ///		@returns : A new StrongPtr to the created object.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // template <typename ... Params> requires nes::ValidConstructorForType<Type, Params...>
    // StrongPtr<Type> StrongPtr<Type>::Create(Params&&...params)
    // {
    //     internal::RefCounter<Type>* pRefCounter = NES_NEW(internal::RefCounter<Type>(std::forward<Params>(params)...)); 
    //     return StrongPtr(pRefCounter);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Add a reference to the internal RefCounter object.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // void StrongPtr<Type>::AddRef()
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter->AddRef();
    //     }
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Remove a refernce to the internal RefCounter object. If there are no more references,
    // ///             the RefCounter object will be deleted and the internal object's destructor will be called.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // void StrongPtr<Type>::RemoveRef()
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         // Removing a reference can trigger the deletion of the object.
    //         m_pRefCounter->RemoveRef();
    //
    //         // If there are no more references (weak or strong) to ReferenceCounter, delete it.
    //         if (m_pRefCounter->HasZeroReferences())
    //         {
    //             NES_DELETE(m_pRefCounter);
    //             m_pRefCounter = nullptr;
    //         }
    //     }
    // }

    // template <typename Type>
    // WeakPtr<Type>::WeakPtr(internal::RefCounter<Type>* pRefCounter)
    //     : m_pRefCounter(pRefCounter)
    // {
    //     AddRef();
    // }
    //
    // template <typename Type>
    // WeakPtr<Type>::WeakPtr(const StrongPtr<Type>& pStrongPtr)
    //     : m_pRefCounter(pStrongPtr.m_pRefCounter)
    // {
    //     AddRef();
    // }
    //
    // template <typename Type>
    // WeakPtr<Type>::WeakPtr(const WeakPtr& other)
    // {
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = other.m_pRefCounter;
    //         AddRef();
    //     }
    // }
    //
    // template <typename Type>
    // WeakPtr<Type>::WeakPtr(WeakPtr&& other) noexcept
    // {
    //     // Steal the pointer, don't add a reference.
    //     m_pRefCounter = other.m_pRefCounter;
    //     other.m_pRefCounter = nullptr;
    // }
    //
    // template <typename Type>
    // WeakPtr<Type>& WeakPtr<Type>::operator=(const WeakPtr& other)
    // {
    //     if (this != &other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         m_pRefCounter = other.m_pRefCounter;
    //         AddRef();
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // WeakPtr<Type>& WeakPtr<Type>::operator=(WeakPtr&& other) noexcept
    // {
    //     if (this != &other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         // Steal the pointer, but don't add a weak reference.
    //         m_pRefCounter = other.m_pRefCounter;
    //         other.m_pRefCounter = nullptr;
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // WeakPtr<Type>::~WeakPtr()
    // {
    //     RemoveRef();
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<Type>& WeakPtr<Type>::operator=(const StrongPtr<OtherType>& other)
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         RemoveRef();
    //     }
    //
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = &(other.m_pRefCounter->Cast<Type>());
    //         AddRef();
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<Type>::WeakPtr(const WeakPtr<OtherType>& other)
    // {
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //         AddRef();
    //     }
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<Type>::WeakPtr(const StrongPtr<OtherType>& other)
    // {
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //         AddRef();
    //     }
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<Type>::WeakPtr(StrongPtr<OtherType>&& other) noexcept
    // {
    //     if (other.m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //         other.m_pRefCounter = nullptr;
    //     }
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<Type>& WeakPtr<Type>::operator=(const WeakPtr<OtherType>& other)
    // {
    //     if (*this != other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         if (other.m_pRefCounter != nullptr)
    //         {
    //             m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //             AddRef();
    //         }
    //     }
    //
    //     return *this;
    // }
    //
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<Type>& WeakPtr<Type>::operator=(WeakPtr<OtherType>&& other) noexcept
    // {
    //     if (*this != other)
    //     {
    //         if (m_pRefCounter != nullptr)
    //         {
    //             RemoveRef();
    //         }
    //
    //         // Steal the pointer, but don't add a weak reference.
    //         m_pRefCounter = &(other.m_pRefCounter->template Cast<Type>());
    //         other.m_pRefCounter = nullptr;
    //     }
    //
    //     return *this;
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //		
    // ///		@brief : Create a StrongPtr reference to the Object that this WeakPtr points to. This will ensure
    // ///             that the object is not deleted while the StrongPtr is in scope.
    // ///		@returns : StrongPtr to the object that this WeakPtr points to.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // StrongPtr<Type> WeakPtr<Type>::Lock() const
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         // TODO: I need friend access...
    //         return StrongPtr<Type>(m_pRefCounter);
    //     }
    //
    //     return StrongPtr<Type>();
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Returns true if the Object pointed to by the WeakPtr is still valid.
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // bool WeakPtr<Type>::IsValid() const
    // {
    //     return m_pRefCounter != nullptr && m_pRefCounter->Get() != nullptr;
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //		
    // ///		@brief : Cast this WeakPtr to a WeakPtr of a different type.
    // ///		@tparam OtherType : Other Type to cast to. The underlying object that this points to must be
    // ///                        of type OtherType.
    // ///		@returns : 
    // //----------------------------------------------------------------------------------------------------
    // template <typename Type>
    // template <typename OtherType> requires nes::TypeIsBaseOrDerived<Type, OtherType>
    // WeakPtr<OtherType> WeakPtr<Type>::Cast() const
    // {
    //     return WeakPtr<OtherType>(*this);
    // }
    //
    // template <typename Type>
    // void WeakPtr<Type>::AddRef()
    // {
    //     if (m_pRefCounter)
    //         m_pRefCounter->AddWeak();
    // }
    //
    // template <typename Type>
    // void WeakPtr<Type>::RemoveRef()
    // {
    //     if (m_pRefCounter != nullptr)
    //     {
    //         m_pRefCounter->RemoveWeak();
    //
    //         // If there are no more references (weak or strong) to ReferenceCounter, delete it.
    //         if (m_pRefCounter->HasZeroReferences())
    //         {
    //             NES_DELETE(m_pRefCounter);
    //             m_pRefCounter = nullptr;
    //         }
    //     }
    // }
}

#include "StrongPtr.inl"