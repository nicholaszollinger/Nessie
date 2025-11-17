// EditorWorld.h
#pragma once
#include "Nessie/World.h"
#include "Nessie/Asset/AssetManager.h"

namespace nes
{
    class World;

    //----------------------------------------------------------------------------------------------------
    /// @brief : The Editor World manages simulating a runtime world. It does not have an entity registry
    /// itself - when not simulating, the WorldAsset's entity registry is used, and when simulating the runtime world's
    /// entity registry is used.
    //----------------------------------------------------------------------------------------------------
    class EditorWorld final : public WorldBase
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Runtime World Instance that has the renderer and update logic. 
        //----------------------------------------------------------------------------------------------------
        void                                SetRuntimeWorld(const StrongPtr<World>& pWorld);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the World Asset that is edited when not simulating and is copied into the runtime world
        ///    for simulation.
        //----------------------------------------------------------------------------------------------------
        void                                SetWorldAsset(const AssetID worldAssetID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current World Asset that we are using.
        //----------------------------------------------------------------------------------------------------
        AssetPtr<WorldAsset>                GetCurrentWorldAsset() const { return m_pCurrentWorldAsset; }
        
        // World Base API
        virtual void                        Tick(const float deltaTime) override;
        virtual void                        OnEvent(Event& event) override;
        virtual EntityHandle                CreateEntity(const std::string& name) override;
        virtual void                        DestroyEntity(const EntityHandle entity) override;
        virtual void                        ParentEntity(const EntityHandle entity, const EntityHandle parent) override;
        
        virtual EntityRegistry*             GetEntityRegistry() override;
        virtual StrongPtr<WorldRenderer>    GetRenderer() const override;
        virtual StrongPtr<ComponentSystem>  GetSystem(const entt::id_type typeID) const override;
    
    private:
        virtual void                        AddComponentSystems() override;
        virtual bool                        PostInit() override;
        virtual void                        OnDestroy() override;
        virtual void                        OnBeginSimulation() override;
        virtual void                        OnEndSimulation() override;

    private:
        StrongPtr<World>                    m_pRuntimeWorld = nullptr;
        AssetPtr<WorldAsset>                m_pCurrentWorldAsset = nullptr;
    };
}
