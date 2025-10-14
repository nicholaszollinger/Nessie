// RefCounter.h
#pragma once
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Debug/CheckedCast.h"
#undef GetObject

namespace nes::internal
{
    class RefCounterBase
    {
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
        uint32              GetRefCount() const        { return m_refCount.load(std::memory_order_relaxed); }

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
        /// A Large value that is added to the ref count, so that it remains in memory when the number of
        /// external references becomes zero.
        static constexpr uint32 kEmbedded = 0x0ebedded;
        
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
        //----------------------------------------------------------------------------------------------------
        /// @note : IMPORTANT! The pointer parameter will be set to null by this constructor! This is taking
        ///     ownership of the memory!
        //----------------------------------------------------------------------------------------------------
        explicit            RefCounter(Type*& pObject) : m_pObject(pObject) { pObject = nullptr; }

        template <typename To>
        RefCounter<To>*     GetAs();

        template <typename To>
        RefCounter<To>*     TryGetAs();

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

#include "RefCounter.inl"