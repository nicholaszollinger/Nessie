// AssetManager.cpp
#include "AssetManager.h"

namespace nes
{
    static AssetManager* g_pInstance = nullptr;

    AssetManager::AssetManager()
    {
        NES_ASSERT(g_pInstance == nullptr);
        g_pInstance = this;
    }

    AssetManager::~AssetManager()
    {
        NES_ASSERT(g_pInstance == this);
        g_pInstance = nullptr;
    }

    void AssetManager::FreeAsset(const AssetID& id)
    {
        AssetManager& assetManager = Get();

        if (auto it = assetManager.m_idToInfoMap.find(id); it != assetManager.m_idToInfoMap.end())
        {
            auto& info = it->second;

            NES_ASSERT(assetManager.m_assetPools.contains(info.m_typeID));
            auto& assetPool = *(assetManager.m_assetPools.at(info.m_typeID));

            // Call Free for the asset.
            const AssetHandle handle = info.m_handle;
            {
                AssetBase* pAsset = assetPool.GetAsset(handle);
                pAsset->Free();
            }

            // Destruct the freed asset.
            assetPool.DestructAsset(handle);
        }

        NES_WARN("Attempted to free an invalid asset! ID: ", id.GetValue());
    }

    AssetID AssetManager::GenerateAssetID()
    {
        static UUIDGenerator generator{};
        return generator.GenerateUUID();
    }

    AssetManager& AssetManager::Get()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return *g_pInstance;
    }

    void AssetManager::Shutdown()
    {
        NES_ASSERT(g_pInstance != nullptr);

        // Free all remaining assets:
        // [TODO]: It would be nice to Free in a single batch.
        // - I could do something like BeginBatch(), AddToCurrentBatch(), SubmitBatch() that would internally
        //   create a batch and track it. Iterating through here, I am not guaranteed to run through a block of
        //   assets at a time.
        
        for (const auto& [_, info] : m_idToInfoMap)
        {
            NES_ASSERT(m_assetPools.contains(info.m_typeID));
            
            auto* pPoolBase = m_assetPools.at(info.m_typeID);

            // Free the Asset:
            {
                AssetBase* pAsset = pPoolBase->GetAsset(info.m_handle);
                pAsset->Free();
            }

            // Then destruct.
            pPoolBase->DestructAsset(info.m_handle);
        }

        m_idToInfoMap.clear();

        // Delete all pools:
        for (auto& [typeID, pPool] : m_assetPools)
        {
            NES_SAFE_DELETE(pPool);
        }

        m_assetPools.clear();
    }

}