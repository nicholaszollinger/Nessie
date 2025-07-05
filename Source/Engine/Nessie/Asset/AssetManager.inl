// AssetManager.inl
#pragma once

namespace nes
{
    template <ValidAssetType Type>
    StrongPtr<Type> AssetManager::LoadAssetFromPath(const std::filesystem::path& path)
    {
        AssetManager& assetManager = Get();
        const AssetID id = GenerateAssetIDFromPath(path);
        auto& assetPool = assetManager.GetAssetPool<Type>();

        // Asset loaded already, return it.
        if (auto it = assetManager.m_idToInfoMap.find(id); it != assetManager.m_idToInfoMap.end())
        {
            NES_ASSERT(it->second.m_typeID == Type::GetStaticTypeID());
            AssetBase* pAsset = assetPool.GetAsset(it->second.m_handle);
            return StrongPtr<Type>(pAsset);
        }

        // Otherwise, we need to load it. Do it now.
        AssetHandle handle = assetPool.ConstructAsset();
        if (handle == kInvalidAssetHandle)
            return nullptr; // Out of space!

        AssetBase* pAsset = &assetPool.Get(handle);
        if (!pAsset->LoadFromFile(path))
        {
            // Failed to load.
            assetPool.Destruct(handle);
            return nullptr;
        }

        // Load succeeded!
        assetManager.m_idToInfoMap[id] = AssetInfo(Type::GetStaticTypeID(), handle);

        // Set the handle and id of the asset.
        pAsset->m_id = id;
        pAsset->m_handle = handle;
        return StrongPtr<Type>(pAsset);
    }

    template <ValidAssetType Type>
    StrongPtr<Type> AssetManager::AddMemoryAsset(Type&& asset, AssetID overrideID)
    {
        AssetManager& assetManager = Get();

        // If the passed in value is invalid, then we need to generate a new ID.
        if (overrideID == kInvalidAssetID)
        {
            overrideID = GenerateAssetID();
        }

        auto& assetPool = assetManager.GetAssetPool<Type>();
        AssetHandle handle = assetPool.ConstructAsset(std::forward<Type>(asset));
        if (handle == kInvalidAssetHandle)
            return nullptr; // Out of space!

        // Success!
        AssetBase* pAsset = &assetPool.Get(handle);
        assetManager.m_idToInfoMap[overrideID] = AssetInfo(Type::GetStaticTypeID(), handle);

        // Set the handle and ID of the asset.
        pAsset->m_id = overrideID;
        pAsset->m_handle = handle;
        
        return StrongPtr<Type>(pAsset);
    }

    template <ValidAssetType Type>
    StrongPtr<Type> AssetManager::GetAsset(const AssetID& id)
    {
        AssetManager& assetManager = Get();
        auto& assetPool = assetManager.GetAssetPool<Type>();

        if (auto it = assetManager.m_idToInfoMap.find(id); it != assetManager.m_idToInfoMap.end())
        {
            NES_ASSERT(it->second.m_typeID == Type::GetStaticTypeID());
            const AssetHandle handle = it->second.m_handle;
            return StrongPtr<Type>(&assetPool.Get(handle));
        }

        return nullptr;
    }

    template <ValidAssetType Type>
    void AssetManager::InitializePool(const AssetPoolCreateInfo& info)
    {
        const TypeID assetTypeID = Type::GetStaticTypeID();
        
        if (!m_assetPools.contains(assetTypeID))
        {
            AssetPool<Type>* pNewPool = NES_NEW(AssetPool<Type>());
            pNewPool->Init(info);
            m_assetPools[assetTypeID] = pNewPool;
        }

        else
        {
            NES_WARN("Tried to initialize an AssetPool of type '{}' twice!", Type::GetStaticTypename());
        }
    }

    template <ValidAssetType Type>
    AssetPool<Type>& AssetManager::GetAssetPool()
    {
        const TypeID assetTypeID = Type::GetStaticTypeID();

        // Return the pool for that type if one is already present.
        if (auto it = m_assetPools.find(assetTypeID); it != m_assetPools.end())
        {
            return *checked_cast<AssetPool<Type>*>(it->second);
        }

        // Otherwise, create a default pool of that type.
        AssetPool<Type>* pNewPool = NES_NEW(AssetPool<Type>());
        pNewPool->Init(AssetPoolCreateInfo());
        m_assetPools[assetTypeID] = pNewPool;
        return *pNewPool;
    }
}