// IDComponent.cpp
#include "IDComponent.h"
#include <unordered_set>
#include "Nessie/Core/Thread/Mutex.h"
#include "Nessie/Random/Rng.h"

namespace nes
{
    static EntityID GenerateUniqueID()
    {
        static Mutex idGeneratorMutex{};
        static Rng idGenerator{};
        static std::unordered_set<EntityID> idSet{};

        std::unique_lock idLock(idGeneratorMutex);
        EntityID newID = idGenerator.RandRange(1ull, std::numeric_limits<uint64_t>::max());
        while (idSet.contains(newID))
        {
            newID = idGenerator.RandRange(1ull, std::numeric_limits<uint64_t>::max());
        }
        idSet.emplace(newID);
        return newID;
    }
    
    IDComponent::IDComponent(const std::string& name)
        : m_name(name)
    {
        m_id = GenerateUniqueID();
    }

    // IDComponent::IDComponent(const IDComponent& other)
    //     : m_name(other.m_name)
    // {
    //     m_id = GenerateUniqueID();
    // }
    //
    // IDComponent& IDComponent::operator=(const IDComponent& other)
    // {
    //     if (this != &other)
    //     {
    //         m_name = other.m_name;
    //         m_id = GenerateUniqueID();
    //     }
    //
    //     return *this;
    // }

    IDComponent::IDComponent(IDComponent&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_id(other.m_id)
    {
        other.m_id = kInvalidEntityID;
    }

    IDComponent& IDComponent::operator=(IDComponent&& other) noexcept
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_name = std::move(other.m_name);
            
            other.m_id = kInvalidEntityID;
        }

        return *this;
    }
}
