// AssetPool.h
#pragma once
#include "AssetBase.h"
#include "Nessie/Core/Memory/FixedSizedFreeList.h"

namespace nes
{
    struct AssetPoolCreateInfo
    {
        static constexpr uint32 kDefaultMaxAssets = 64;
        static constexpr uint32 kDefaultPageSize = 64;
        
        uint32 m_maxAssets = kDefaultMaxAssets;
        uint32 m_pageSize = kDefaultPageSize;

        AssetPoolCreateInfo() = default;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for an AssetPool, which defines a TypeID associated with the pool and the API
    ///     used by the AssetManager,  
    //----------------------------------------------------------------------------------------------------
    class AssetPoolBase
    {
    public:
        AssetPoolBase() = default;
        virtual ~AssetPoolBase() = default;

        /// No copy or move.
        AssetPoolBase(const AssetPoolBase&) = delete;
        AssetPoolBase(AssetPoolBase&&) noexcept = delete;
        AssetPoolBase& operator=(const AssetPoolBase&) = delete;
        AssetPoolBase& operator=(AssetPoolBase&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the asset pool.
        //----------------------------------------------------------------------------------------------------
        virtual void        Init(const AssetPoolCreateInfo& info) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get an asset from the pool from an AssetHandle.
        //----------------------------------------------------------------------------------------------------
        virtual AssetBase*  GetAsset(const AssetHandle handle) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destruct an asset with the same AssetHandle. 
        //----------------------------------------------------------------------------------------------------
        virtual void        DestructAsset(const AssetHandle handle) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clear the Asset Pool, destructing all remaining assets. 
        //----------------------------------------------------------------------------------------------------
        virtual void        Clear() = 0;
        
    protected:
        TypeID              m_assetTypeID = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Fixed Sized Free List of a specific type of Asset. 
    //----------------------------------------------------------------------------------------------------
    template <ValidAssetType Type>
    class AssetPool final : public AssetPoolBase
    {
    public:
        using AssetType = Type;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Batch of Assets to be destructed. 
        //----------------------------------------------------------------------------------------------------
        using Batch = typename FixedSizeFreeList<Type>::Batch;

    public:
        AssetPool() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the pool with info.m_maxAssets assets. 
        //----------------------------------------------------------------------------------------------------
        virtual void        Init(const AssetPoolCreateInfo& info) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clear the Asset Pool, destructing all remaining assets. 
        //----------------------------------------------------------------------------------------------------
        virtual void        Clear() override                                    { m_pool.Clear(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a single asset. 
        //----------------------------------------------------------------------------------------------------
        AssetHandle         ConstructAsset()                                    { return m_pool.ConstructObject(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Construct a single asset by moving the 'value' param into the pool.
        //----------------------------------------------------------------------------------------------------
        AssetHandle         ConstructAsset(Type&& value)                        { return m_pool.ConstructObject(std::forward<Type>(value)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destruct a single asset.
        //----------------------------------------------------------------------------------------------------
        virtual void        DestructAsset(const AssetHandle handle) override    { m_pool.DestructObject(handle); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get an asset from the pool, converted to the base type.
        //----------------------------------------------------------------------------------------------------
        virtual AssetBase*  GetAsset(const AssetHandle handle) override         { return &m_pool.Get(handle); }  

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an asset to a batch, to be able to destruct a number of assets all at once.
        //----------------------------------------------------------------------------------------------------
        void                AddAssetToBatch(Batch& batch, AssetHandle handle)   { m_pool.AddObjectToBatch(batch, handle); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destruct a batch of assets. 
        //----------------------------------------------------------------------------------------------------
        void                DestructBatch(Batch& batch)                         { m_pool.DestructBatch(batch); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get an Asset from the handle.
        //----------------------------------------------------------------------------------------------------
        Type&               Get(const AssetHandle& handle)                      { return m_pool.Get(handle); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get an Asset from the handle; const version.
        //----------------------------------------------------------------------------------------------------
        const Type&         Get(const AssetHandle& handle) const                { return m_pool.Get(handle); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check that the Asset Pool has been initialized. 
        //----------------------------------------------------------------------------------------------------
        bool                IsInitialized() const                               { return m_isInitialized; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of assets in the pool. 
        //----------------------------------------------------------------------------------------------------
        size_t              GetAssetCount() const                               { return m_pool.Count(); }

    private:
        FixedSizeFreeList<AssetType> m_pool;
        bool                m_isInitialized = false;
    };

    template <ValidAssetType Type>
    void AssetPool<Type>::Init(const AssetPoolCreateInfo& info)
    {
        m_assetTypeID = Type::GetStaticTypeID();
        m_pool.Init(info.m_maxAssets, info.m_pageSize);
        m_isInitialized = true;
    }
}
