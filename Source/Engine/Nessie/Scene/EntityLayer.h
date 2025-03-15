// EntityLayer.h
#pragma once
#include "Entity.h"

#define NES_DEFINE_ENTITY_LAYER(layerTypename, entityTypename)    \
NES_DEFINE_TYPE_INFO(layerTypename)                               \
public:                                                           \
    using EntityType = entityTypename;                            \
private:

namespace YAML { class Node; }

namespace nes
{
    class Camera;
    class Event;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A EntityLayer manages a type of SceneNode in the Scene. For example,
    ///         the World is a EntityLayer that manages Actor Nodes, which exist in 3D space.
    //----------------------------------------------------------------------------------------------------
    class EntityLayer
    {
        friend class Scene;
        
    protected:
        Scene* m_pScene = nullptr;
        bool m_isBeingDestroyed = false;
        
    public:
        explicit EntityLayer(Scene* pScene) : m_pScene(pScene) {}
        virtual ~EntityLayer() = default;
        EntityLayer(const EntityLayer&) = delete;
        EntityLayer& operator=(const EntityLayer&) = delete;
        EntityLayer(EntityLayer&&) noexcept = delete;
        EntityLayer& operator=(EntityLayer&&) noexcept = delete;

        virtual void DestroyEntity(const LayerHandle& handle) = 0;

        [[nodiscard]] virtual TypeID        GetTypeID() const = 0;
        [[nodiscard]] virtual const char*   GetTypename() const = 0;
        [[nodiscard]] virtual bool          IsValidNode(const LayerHandle& handle) const = 0;
        [[nodiscard]] Scene*                GetScene() const            { return m_pScene; }
        [[nodiscard]] bool                  IsBeingDestroyed() const    { return m_isBeingDestroyed; }

    private:
        void DestroyLayer();
        
        virtual bool InitializeLayer() = 0;
        virtual void OnSceneBegin() = 0;
        virtual void OnEvent(Event& event) = 0;
        virtual void Render(const Camera& sceneCamera) = 0;
        virtual void Tick(const double deltaTime) = 0;
        virtual void OnLayerDestroyed() = 0;

        virtual bool LoadLayer(YAML::Node& layerNode) = 0;
        virtual void EditorRenderEntityHierarchy() = 0;
    };

    template <typename Type>
    concept EntityLayerType = !std::is_abstract_v<Type> && requires(Type layer)
    {
        TypeIsDerivedFrom<Type, EntityLayer>;
        ValidEntityType<typename Type::EntityType>;
    };
}
