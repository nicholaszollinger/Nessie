// WorldAsset.cpp
#include "WorldAsset.h"

#include <fstream>
#include <ranges>
#include "Nessie/World.h"
#include "Nessie/Core/String/StringID.h"

namespace nes
{
    WorldAsset::WorldAsset(WorldAsset&& other) noexcept
        : m_entityRegistry(std::move(other.m_entityRegistry))
        , m_rootEntities(std::move(other.m_rootEntities))
        , m_assetPack(std::move(other.m_assetPack))
    {
        //
    }

    WorldAsset& WorldAsset::operator=(WorldAsset&& other) noexcept
    {
        if (this != &other)
        {
            m_entityRegistry = std::move(other.m_entityRegistry);
            m_rootEntities = std::move(other.m_rootEntities);
            m_assetPack = std::move(other.m_assetPack);
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

    void WorldAsset::SaveToFile(const std::filesystem::path& path)
    {
        // [TODO]: Verify that the extension is a yaml file.

        std::ofstream stream(path.string());
        if (!stream.is_open())
        {
            NES_ERROR("Failed to save World Asset! Failed to open filepath: {}", path.string());
            return;
        }
        
        YamlOutStream out(path, stream);
        NES_ASSERT(out.IsOpen());

        out.BeginMap("World");

        // Serialize the Assets:
        AssetPack::Serialize(out, m_assetPack);

        // Serializer the Entities:
        auto& componentRegistry = ComponentRegistry::Get();
        auto componentTypes = componentRegistry.GetAllComponentTypes();
        
        // Remove non-serializable components:
        for (size_t i = 0; i < componentTypes.size();)
        {
            if (!componentTypes[i].m_serializeYAML)
            {
                std::swap(componentTypes[i], componentTypes.back());
                componentTypes.pop_back();
                continue;
            }
            
            ++i;
        }

        // Save all entities in root entity order.
        // - All children of the root entity will be saved
        out.BeginSequence("Entities");
        for (auto entityID : m_rootEntities)
        {
            const auto entity = m_entityRegistry.GetEntity(entityID);
            SaveEntityAndChildren(entity, componentTypes, out);
        }
        out.EndSequence(); // End "Entities" sequence.
        
        out.EndMap(); // End "World" map.
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

            auto* pNodeComponent = m_entityRegistry.TryGetComponent<NodeComponent>(entity);
            if (!pNodeComponent || pNodeComponent->m_parentID == kInvalidEntityID)
            {
                // This entity is a root entity, add it to the end of the array.
                m_rootEntities.emplace_back(entityLoadID);
            }
        }
        
        return true;
    }

    void WorldAsset::SaveEntityAndChildren(const EntityHandle entity, const std::vector<ComponentTypeDesc>& componentTypes, YamlOutStream& out)
    {
        NES_ASSERT(entity != kInvalidEntityHandle, "Invalid child found when saving world!");
        out.BeginMap();
        
        // IDComponent information:
        auto& idComp = m_entityRegistry.GetComponent<IDComponent>(entity);
        out.Write("Entity", idComp.GetID());
        out.Write("Name", idComp.GetName());

        // [TODO]: Enabled state.

        // Save all Components.
        out.BeginSequence("Components");
        for (auto& componentType : componentTypes)
        {
            NES_ASSERT(componentType.m_serializeYAML);
            componentType.m_serializeYAML(out, m_entityRegistry, entity);
        }
        out.EndSequence();
            
        out.EndMap(); // End "Entity" Map.

        // Save all children data, recursively:
        if (auto* pNodeComponent = m_entityRegistry.TryGetComponent<NodeComponent>(entity))
        {
            for (const auto childID : pNodeComponent->m_childrenIDs)
            {
                const auto child = m_entityRegistry.GetEntity(childID);
                SaveEntityAndChildren(child, componentTypes, out);
            }
        }
    }   
}
