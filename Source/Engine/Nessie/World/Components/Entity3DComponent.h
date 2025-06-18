// ActorComponent.h
#pragma once
#include "Scene/Component.h"
#include "Scene/TickFunction.h"

namespace nes
{
    class Entity3D;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Components that can be attached to an Entity that exist in 3D space. 
    //----------------------------------------------------------------------------------------------------
    class Entity3DComponent : public TComponent<Entity3D>
    {
        NES_DEFINE_COMPONENT_TYPE(Entity3DComponent)
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Tick Function for TickableEntity3DComponents.
    //----------------------------------------------------------------------------------------------------
    class Entity3DComponentTickFunction final : public TickFunction
    {
    public:
        class TickableEntity3DComponent* m_pTarget = nullptr;
        bool m_tickWhilePaused = false;
        bool m_startWithTickEnabled = true;
        
        Entity3DComponentTickFunction() = default;
        virtual void    ExecuteTick(const TickDeltaTime& deltaTime) override;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : A TickableEntity3DComponent is a base class for any component that can attach to a 3D Entity
    ///             that you want to Tick. If your component doesn't need to tick at all, you can use the
    ///             Entity3DComponent base class instead.
    //----------------------------------------------------------------------------------------------------
    class TickableEntity3DComponent : public Entity3DComponent
    {
        NES_DEFINE_COMPONENT_TYPE(TickableEntity3DComponent)

    public:
        using TickFunction = Entity3DComponentTickFunction;
        
    public:
        TickableEntity3DComponent() = default;
        
        virtual void    Tick(const float deltaTime) = 0;
        void            SetTickEnabled(bool enabled);
        void            RegisterTickToGroup(TickGroup* pTickGroup);
    
    protected:
        virtual bool    Init() override;
        virtual void    OnEnabled() override;
        virtual void    OnDisabled() override;

    protected:
        TickFunction m_tickFunction;
    };
}
