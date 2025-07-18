// RendererResource.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Core/Thread/Atomics.h"

//-------------------------------------------------------------------------------------------------
// Under development. This file is going to define how Graphics Resources are handled. I have looked
// at a couple of libraries (this is similar to NVRHI), but I will probably change this. 
//-------------------------------------------------------------------------------------------------


namespace nes
{
    /// "Graphics Object Type"
    using GObjectType = uint32;

    enum class EGraphicsObjectType : uint32
    {
        Unknown = 0,
        Instance,
        PhysicalDevice,
        LogicalDevice,
        Queue,
        CommandBuffer,
        DeviceMemory,
        Buffer,
        Image,
        ImageView,
        Sampler,
        Shader,
        Framebuffer,
        RenderPass,
        Pipeline,
        PipelineLayout,
        DescriptorPool,
        DescriptorSetLayout,
        DescriptorSet,
        AccelerationStructure,
        MicroMap,
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : "Graphics Object".
    //----------------------------------------------------------------------------------------------------
    struct GObject
    {
        union
        {
            uint64  m_integer;
            void*   m_pointer;
        };

        GObject(const uint64 val) : m_integer(val) {}
        GObject(void* val)        : m_pointer(val) {}

        template <typename Type>
        operator Type*() const { return static_cast<Type*>(m_pointer); }
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : "Graphics Resource".
    //----------------------------------------------------------------------------------------------------
    class GResource
    {
    public:
        // No copy or move.
        GResource(const GResource&) = delete;
        GResource& operator=(const GResource&) = delete;
        GResource(GResource&&) noexcept = delete;
        GResource& operator=(GResource&&) noexcept = delete;

        virtual uint32 AddRef() = 0;
        virtual uint32 Release() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a native graphics object or interface (ex. VulkanDevice*), or nullptr if the requested
        ///     interface is unavailable.
        //----------------------------------------------------------------------------------------------------
        virtual GObject GetNativeObject([[maybe_unused]] GObjectType type) { return nullptr; }

    protected:
        // Protected Construction/Destruction.
        GResource() = default;
        virtual ~GResource() = default;
    };

    template <typename Type>
    concept GraphicsResourceType = nes::TypeIsBaseOrDerived<Type, GResource>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : "Graphics Resource Pointer". Calls "Release()" on the object 
    //----------------------------------------------------------------------------------------------------
    template <GraphicsResourceType Type> 
    class GResourcePtr
    {
    public:
        using InterfaceType = Type;

    public:
        GResourcePtr() noexcept : m_ptr(nullptr) {}
        GResourcePtr(std::nullptr_t) noexcept : m_ptr(nullptr) {}

        template <GraphicsResourceType Other>
        GResourcePtr(Other* pOther) noexcept
            : m_ptr(pOther)
        {
            InternalAddRef();
        }

        GResourcePtr(const GResourcePtr& other) noexcept
            : m_ptr(other.m_ptr)
        {
            InternalAddRef();
        }

        template <typename Other> requires std::is_convertible_v<Other*, Type*>
        GResourcePtr(const GResourcePtr<Other>& other) noexcept
            : m_ptr(other.m_ptr)
        {
            InternalAddRef();
        }

        GResourcePtr(GResourcePtr&& other) noexcept
            : m_ptr(nullptr)
        {
            if (this != &other)
            {
                Swap(other);
            }
        }

        template <typename Other> requires std::is_convertible_v<Other*, Type*>
        GResourcePtr(GResourcePtr<Other>&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~GResourcePtr() noexcept
        {
            InternalRelease();
        }

        GResourcePtr& operator=(std::nullptr_t) noexcept
        {
            InternalRelease();
            return *this;
        }

        GResourcePtr& operator=(Type* pOther) noexcept
        {
            if (m_ptr != pOther)
            {
                GResourcePtr(pOther).Swap(*this);
            }
            return *this;
        }

        template <GraphicsResourceType Other>
        GResourcePtr& operator=(Other* pOther) noexcept
        {
            GResourcePtr(pOther).Swap(*this);
            return *this;
        }

        GResourcePtr& operator=(const GResourcePtr& other) noexcept
        {
            if (m_ptr != other.m_ptr)
            {
                GResourcePtr(other).Swap(*this);
            }
            return *this;
        }

        template <GraphicsResourceType Other>
        GResourcePtr& operator=(const GResourcePtr<Other>& other) noexcept
        {
            GResourcePtr(other).Swap(*this);
            return *this;
        }

        GResourcePtr& operator=(GResourcePtr&& other) noexcept
        {
            GResourcePtr(static_cast<GResourcePtr&&>(other)).Swap(*this);
            return *this;
        }

        template <GraphicsResourceType Other>
        GResourcePtr& operator=(GResourcePtr<Other>&& other) noexcept
        {
            GResourcePtr(static_cast<GResourcePtr<Other>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(GResourcePtr&& other) noexcept
        {
            Type* pTemp = m_ptr;
            m_ptr = other.m_ptr;
            other.m_ptr = pTemp;
        }

        void Swap(GResourcePtr& other) noexcept
        {
            Type* pTemp = m_ptr;
            m_ptr = other.m_ptr;
            other.m_ptr = pTemp;
        }

        /// Operators
        operator Type*() const                          { return m_ptr; }
        InterfaceType* operator->() const noexcept      { return m_ptr; }
        Type** operator&()                              { return &m_ptr; }
    
        [[nodiscard]] Type* Get() const noexcept
        {
            return m_ptr;
        }
    
        [[nodiscard]] Type* const* GetAddressOf() const noexcept
        {
            return &m_ptr;
        }
    
        [[nodiscard]] Type** GetAddressOf() noexcept
        {
            return &m_ptr;
        }
    
        [[nodiscard]] Type** ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &m_ptr;
        }

        Type* Detach() noexcept
        {
            Type* ptr = m_ptr;
            m_ptr = nullptr;
            return ptr;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pointer while keeping the resource's reference count unchanged. 
        //----------------------------------------------------------------------------------------------------
        void Attach(InterfaceType* other)
        {
            if (m_ptr != nullptr)
            {
                [[maybe_unused]] auto ref = m_ptr->Release();

                // Attaching to the same object only works if duplicate references are being coalesced. Otherwise,
                // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
                NES_ASSERT(ref != 0 && m_ptr != other);
            }

            m_ptr = other;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Release this resource. Returns the remaining ref count.
        //----------------------------------------------------------------------------------------------------
        uint32 Reset()
        {
            return InternalRelease();
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new Graphics Resource Ptr for the value.
        //----------------------------------------------------------------------------------------------------
        static GResourcePtr Create(Type* pValue)
        {
            GResourcePtr ptr;
            ptr.Attach(pValue);
            return ptr;
        }

    protected:
        template <typename Other> friend class GResourcePtr;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a reference to this resource. 
        //----------------------------------------------------------------------------------------------------
        void InternalAddRef() const noexcept
        {
            if (m_ptr != nullptr)
                m_ptr->AddRef();
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Release this resource. Returns the remaining reference count.
        //----------------------------------------------------------------------------------------------------
        uint32 InternalRelease() noexcept
        {
            uint32 ref = 0;
            Type* pTemp = m_ptr;

            if (pTemp != nullptr)
            {
                m_ptr = nullptr;
                ref = pTemp->Release();
            }

            return ref;
        }

    protected:
        InterfaceType* m_ptr;
    };

    using GResourceHandle = GResourcePtr<GResource>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A class that implements reference counting in a way that is compatible with GResourcePtr.
    ///     Intended usage is to use it as a base class for interface implementations.
    ///
    ///     Example:
    ///          class Texture : public GRefCounter<ITexture> { ... }
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class GRefCounter : public Type
    {
    public:
        virtual uint32 AddRef() override
        {
            return ++m_refCount;
        }
    
        virtual uint32 Release() override
        {
            const uint32 result = --m_refCount;
            if (result == 0)
            {
                NES_SAFE_DELETE(this);
            }

            return result;
        }
    
    private:
        std::atomic<uint32> m_refCount = 1;
    };
}