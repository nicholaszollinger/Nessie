// EntityLayer.h
#pragma once
#include "EntityPool.h"
#include "Core/Memory/StrongPtr.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      Ideally, there would be a way that the EntityDomain can be accessed statically from the EntityType
//      provided. However, in the case of an Actor that exists in 3D space, all derived actors must adhere
//      to that space. I don't want to have to redefine the Domain when it isn't necessary, and I can't grab
//      the parent class easily in C++. So I am just having the user define it again for the layer.
//		
///		@brief : Defines requisite information about an Entity Layer, including the type of Entity
///         that this layer manages, and what Domain this layer exists in. The Domain should match
///         the Domain of the Entity that is managed.
///		@param layerTypename : The typename of the Layer class you are defining. 
///		@param entityTypename : The typename of the Entity that this Layer manages. 
///     @param entityDomain : The Entity Domain that the Entities exist in on this Layer.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_ENTITY_LAYER(layerTypename, entityTypename, entityDomain)        \
    NES_DEFINE_TYPE_INFO(layerTypename)                                             \
public:                                                                             \
    using EntityType = entityTypename;                                              \
    static constexpr EntityDomain GetStaticEntityDomain() { return entityDomain; }  \
    virtual EntityDomain GetEntityDomain() const override { return entityDomain; }  \
private:

namespace YAML { class Node; }

namespace nes
{
    class Camera;
    class Event;

    //----------------------------------------------------------------------------------------------------
    ///		@brief : 
    //----------------------------------------------------------------------------------------------------
    class EntityLayer
    {
        friend class Scene;
        
    protected:
        Scene* m_pScene = nullptr;
    
    public:
        explicit EntityLayer(Scene* pScene);
        virtual ~EntityLayer() = default;
        EntityLayer(const EntityLayer&) = delete;
        EntityLayer& operator=(const EntityLayer&) = delete;
        EntityLayer(EntityLayer&&) noexcept = delete;
        EntityLayer& operator=(EntityLayer&&) noexcept = delete;

    public:
        virtual void DestroyEntity(const LayerHandle& handle) = 0;
        
        [[nodiscard]] virtual TypeID        GetTypeID() const = 0;
        [[nodiscard]] virtual const char*   GetTypename() const = 0;
        [[nodiscard]] virtual EntityDomain  GetEntityDomain() const = 0;
        [[nodiscard]] Scene*                GetScene() const { return m_pScene; }
        [[nodiscard]] virtual bool          IsValidEntity(const LayerHandle& handle) const = 0;
    
    private:
        virtual bool InitializeLayer() = 0;
        virtual void OnSceneBegin() = 0;
        virtual void OnEvent(Event& event) = 0;
        virtual void Render(const Camera& sceneCamera) = 0;
        virtual void Tick(const double deltaTime) = 0;
        virtual void DestroyLayer() = 0;
        
        virtual bool LoadLayer(YAML::Node& layerNode) = 0;
        virtual void RenderEditorEntityHierarchy() = 0;
    };

    template <typename Type>
    concept EntityLayerType = !std::is_abstract_v<Type> && requires(Type layer)
    {
        TypeIsDerivedFrom<Type, EntityLayer>;
        // Must define an Entity Class Type that exists on the Layer.
        TypeIsSameOrDerived<typename Type::EntityType, Entity>;

        { layer.GetStaticEntityDomain() } -> std::same_as<EntityDomain>;
    };
}
