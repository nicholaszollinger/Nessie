// AssetManager.inl
#pragma once
#include "AssetBase.h"

namespace nes
{
    template <ValidAssetType Type>
    void LoadRequest::AppendLoad(AssetID& id, const std::filesystem::path& path)
    {
        NES_ASSERT(m_pAssetManager != nullptr);

        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }

        const TypeID typeID = Type::GetStaticTypeID();

        // Register if not already:
        if (!m_pAssetManager->IsTypeRegistered(typeID))
        {
            AssetManager::RegisterAssetType<Type>(StripNamespaceFromTypename(Type::GetStaticTypename()));
        }

        // Create the metadata for the asset.
        AssetMetadata metadata;
        metadata.m_assetID = id;
        metadata.m_path = path;
        metadata.m_typeID = typeID;

        AppendLoad(metadata);
    }

    template <ValidAssetType Type>
    void AssetManager::RegisterAssetType(const std::string& name)
    {
        auto& assetManager = GetInstance();
        
        // Locked check if the type is registered:
        if (assetManager.IsTypeRegistered(Type::GetStaticTypeID()))
            return;

        AssetTypeDesc typeDesc;
        typeDesc.m_type = Type::GetStaticTypeID();
        typeDesc.m_typename = name;
        
        typeDesc.m_createNewAsset = []() -> AssetBase*
        {
            return NES_NEW(Type());            
        };

        typeDesc.m_createNewAssetMove = [](AssetBase&& rValue) -> AssetBase*
        {
            return NES_NEW(Type(std::move(static_cast<Type&&>(rValue))));
        };
        
        typeDesc.m_isRegistered = true;

        // Set the type desc.
        assetManager.SetTypeDesc(std::move(typeDesc));
        
        NES_LOG(kAssetLogTag, "Registered Asset Type: '{}'", name);
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::LoadSync(AssetID& id, const std::filesystem::path& path)
    {
        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }
        
        AssetManager& instance = GetInstance();

        // Register if not already:
        if (!instance.IsTypeRegistered(Type::GetStaticTypeID()))
        {
            RegisterAssetType<Type>(StripNamespaceFromTypename(Type::GetStaticTypename()));
        }

        // Create the metadata
        AssetMetadata metadata;
        metadata.m_path = path;
        metadata.m_assetID = id;
        metadata.m_typeID = Type::GetStaticTypeID();

    #ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        return instance.MainLoadSync(metadata);
    #else
        if (IsMainThread())
            return instance.MainLoadSync(metadata);

        // Asset Thread:
        return instance.ThreadLoadSync(metadata);
    #endif
    }
    
    template <ValidAssetType Type>
    void AssetManager::LoadAsync(AssetID& id, const std::filesystem::path& path, const LoadRequest::OnAssetLoaded& onComplete)
    {
    #ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        const ELoadResult result = LoadSync<Type>(id, std::filesystem::path(path));
        if (onComplete)
        {
            onComplete(result);
        }
    #else
        // Make sure this is the main thread.
        NES_ASSERT(IsMainThread(), "LoadAsync() can only be called on the Main thread! You should call LoadSync() in an Asset's Load function, because it is a synchronous operation on the Asset Thread");

        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }

        AssetManager& instance = GetInstance();
        
        // Determine if we need to perform the load:
        if (LoadedAssetDesc* pDesc = instance.GetAssetDesc(id))
        {
            switch (pDesc->m_state)
            {
                // We have finished or if we were freeing the asset, mark it as Loaded (this will make sure it isn't freed)
                // and call the OnComplete callback immediately. 
                case EAssetState::Loaded:
                case EAssetState::Freeing:
                {
                    pDesc->m_state = EAssetState::Loaded;

                    if (onComplete)
                    {
                        const AsyncLoadResult result(*pDesc, 1.f);
                        onComplete(result);
                    }
                    return;
                }

                // If invalid or freed, perform the load.
                // We still make a request if we are loading so that the callbacks are handled appropriately.
                case EAssetState::Loading:
                case EAssetState::Invalid:
                case EAssetState::Freed:
                    break;
            }
        }
        
        // Submit a new load request with a single asset.
        LoadRequest request = BeginLoadRequest();
        request.SetOnAssetLoadedCallback(onComplete);
        request.AppendLoad<Type>(id, path);
        SubmitLoadRequest(std::move(request));
    #endif
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::AddMemoryAsset(AssetID& id, Type&& asset)
    {
        AssetManager& assetManager = GetInstance();

        // Ensure that this is not a valid AssetID to begin with.
        NES_ASSERT(!IsValidAsset(id));
        
        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetID();
        }

        const TypeID typeID = Type::GetStaticTypeID();

        // Register the type if not already:
        if (!assetManager.IsTypeRegistered(typeID))
        {
            AssetManager::RegisterAssetType<Type>(StripNamespaceFromTypename(Type::GetStaticTypename()));
        }

        // Create the metadata - memory assets have no path.
        AssetMetadata metadata;
        metadata.m_assetID = id;
        metadata.m_typeID = typeID;

        return assetManager.AddMemoryAsset(metadata, std::move(asset));
    }

    template <ValidAssetType Type>
    AssetPtr<Type> AssetManager::GetAsset(const AssetID& id)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be accessed on the Main Thread!");
        
        AssetManager& assetManager = GetInstance();
        if (auto it = assetManager.m_loadedAssetMap.find(id); it != assetManager.m_loadedAssetMap.end())
        {
            auto& desc = it->second;
            NES_ASSERT(desc.m_metadata.m_typeID == Type::GetStaticTypeID(), "Tried to get a loaded asset that is not the requested type!\n\tRequested type: {}", Type::GetStaticTypename());

            // If we are queued to free, but requested, cancel the free operation.
            // Because we only get assets from the main thread, we won't be freeing an asset while
            // changing this flag.
            if (desc.m_state == EAssetState::Freeing)
            {
                desc.m_state = EAssetState::Loaded;
            }
            
            // If not loaded, then return nullptr.
            else if (desc.m_state != EAssetState::Loaded)
            {
                return nullptr;
            }

            NES_ASSERT(desc.m_loadedIndex < assetManager.m_loadedAssets.size());
            NES_ASSERT(assetManager.m_loadedAssets[desc.m_loadedIndex] != nullptr);
            NES_ASSERT(assetManager.m_loadedAssets[desc.m_loadedIndex]->GetTypeID() == Type::GetStaticTypeID());

            // Otherwise, return a pointer to the asset.
            return AssetPtr<Type>(checked_cast<Type*>(assetManager.m_loadedAssets[desc.m_loadedIndex]));
        }

        return nullptr;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>::AssetPtr(const AssetPtr& other)
        : m_pAsset(other.m_pAsset)
        , m_id(other.m_id)
    {
        AddLock();
    }

    template <ValidAssetType Type>
    AssetPtr<Type>::AssetPtr(AssetPtr&& other) noexcept
        : m_pAsset(other.m_pAsset)
        , m_id(other.m_id)
    {
        other.m_pAsset = nullptr;
        other.m_id = kInvalidAssetID;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>& AssetPtr<Type>::operator=(const AssetPtr& other)
    {
        if (this != &other && m_pAsset != other.m_pAsset)
        {
            RemoveLock();
            m_id = other.m_id;
            m_pAsset = other.m_pAsset;
            AddLock();
        }

        return *this;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>& AssetPtr<Type>::operator=(AssetPtr&& other) noexcept
    {
        if (this != &other)
        {
            RemoveLock();
            m_id = other.m_id;
            m_pAsset = other.m_pAsset;
            
            other.m_pAsset = nullptr;
            other.m_id = kInvalidAssetID;
        }

        return *this;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>& AssetPtr<Type>::operator=(std::nullptr_t)
    {
        RemoveLock();
        m_id = kInvalidAssetID;
        m_pAsset = nullptr;

        return *this;
    }

    template <ValidAssetType Type>
    template <ValidAssetType OtherType> requires TypeIsBaseOrDerived<Type, OtherType>
    AssetPtr<OtherType> AssetPtr<Type>::Cast() const
    {
        return checked_cast<OtherType*>(m_pAsset);
    }

    template <ValidAssetType Type>
    AssetPtr<Type>::AssetPtr(Type* pAsset)
        : m_pAsset(pAsset)
    {
        // Set the ID, if valid.
        if (m_pAsset != nullptr)
            m_id = pAsset->GetAssetID();

        // Add a reference to the Asset.
        AddLock();
    }

    template <ValidAssetType Type>
    void AssetPtr<Type>::AddLock() const
    {
        if (m_pAsset)
        {
            NES_ASSERT(IsValid());
            m_pAsset->AddLock();
        }
    }

    template <ValidAssetType Type>
    void AssetPtr<Type>::RemoveLock() const
    {
        if (m_pAsset)
        {
            NES_ASSERT(IsValid());
            m_pAsset->RemoveLock();
        }
    }
}
