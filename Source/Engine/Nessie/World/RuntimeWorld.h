// World.h
#pragma once
#include "WorldBase.h"

namespace nes
{
    class World : public WorldBase
    {
    public:
        // WorldBase EntityID overloads:
        using WorldBase::ParentEntity;
        using WorldBase::DestroyEntity;
        using WorldBase::RemoveParent;

    public:
        virtual EntityHandle    CreateEntity(const std::string& newName) override final;
        virtual void            DestroyEntity(EntityHandle entity) override final;
        virtual EntityRegistry* GetEntityRegistry() override final;
        void                    SetEntityRegistryOverride(EntityRegistry* pRegistry) { m_pEntityRegistryOverride = pRegistry; }
        
    protected:
        virtual void            OnBeginSimulation() override final;
        virtual void            OnEndSimulation() override final;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : When a new Entity is created, the World can attach any required components. The entity will
        ///     just have an IDComponent on creation. 
        //----------------------------------------------------------------------------------------------------
        virtual void            OnNewEntityCreated(EntityRegistry& registry, const EntityHandle newEntity) = 0;
    
    private:
        EntityRegistry          m_entityRegistry{};
        EntityRegistry*         m_pEntityRegistryOverride = nullptr;
    };
}
