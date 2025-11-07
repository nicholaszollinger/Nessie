// WorldAsset.cpp
#include "WorldAsset.h"
#include "Nessie/World.h"
#include "Nessie/Core/String/StringID.h"

namespace nes
{
    WorldAsset::WorldAsset(WorldAsset&& other) noexcept
        : m_entityRegistry(std::move(other.m_entityRegistry))
    {
        //
    }

    WorldAsset& WorldAsset::operator=(WorldAsset&& other) noexcept
    {
        if (this != &other)
        {
            m_entityRegistry = std::move(other.m_entityRegistry);
        }

        return *this;
    }

    ELoadResult WorldAsset::LoadFromFile(const std::filesystem::path& path)
    {
        YamlInStream file(path);

        if (!file.IsOpen())
        {
            NES_ERROR("Failed to load World Asset! \n- Path: {}", path.string());
            return ELoadResult::InvalidArgument;
        }

        auto world = file.GetRoot()["World"];
        NES_ASSERT(world);
        
        auto assets = world["Assets"];
        if (!assets)
        {
            NES_ERROR("Failed to load World! Missing 'Asset' table!\n- Path: {}", path.string());
            return ELoadResult::Failure;
        }
        
        auto entities = world["Entities"];
        if (!entities)
        {
            NES_ERROR("Failed to load World! Missing 'Entities' table!\n- Path: {}", path.string());
            return ELoadResult::Failure;
        }

        // Load the Assets
        if (!AssetPack::Deserialize(assets, m_assetPack))
            return ELoadResult::Failure;

        // Load the Entities:
        if (!LoadEntities(entities))
            return ELoadResult::Failure;

        return ELoadResult::Success;
    }

    bool WorldAsset::LoadEntities(const YamlNode& entities)
    {
        uint64_t entityLoadID;
        std::string entityName{};
        std::string componentName{};
        entityName.reserve(64);
        componentName.reserve(64);

        auto& componentRegistry = ComponentRegistry::Get();
        
        for (auto entityNode : entities)
        {
            // ID Component Information:
            entityNode["Entity"].Read(entityLoadID);
            entityNode["Name"].Read<std::string>(entityName, "");

            // Create the entity:
            EntityHandle entity = m_entityRegistry.CreateEntity(entityLoadID, entityName);

            // This entity needs to be initialized.
            m_entityRegistry.AddComponent<PendingInitialization>(entity);

            // Initial Enable State
            bool startEnabled;
            entityNode["StartEnabled"].Read(startEnabled, true);
            if (startEnabled)
                m_entityRegistry.AddComponent<PendingEnable>(entity);
            else
                m_entityRegistry.AddComponent<DisabledComponent>(entity);

            auto componentsNode = entityNode["Components"];
            for (const auto& componentMapping : componentsNode)
            {
                // Each item in the list is a mapping with one entry
                for (auto it = componentMapping.begin(); it != componentMapping.end(); ++it)
                {
                    it.Key().Read(componentName);
                    const auto& componentNode = it.Value();

                    const ComponentTypeDesc* pDesc = componentRegistry.GetComponentDescByName(componentName);
                    if (!pDesc || pDesc->m_deserializeYAML == nullptr)
                    {
                        NES_ERROR("Failed to load Component named '{}'! Component Type not registered with ComponentRegistry, or has no Deserialize() function!", componentName);
                        return false;
                    }

                    // Load the Component, adding to the Entity.
                    pDesc->m_deserializeYAML(componentNode, m_entityRegistry, entity);
                }
            }
        }
        
        return true;
    }
}
