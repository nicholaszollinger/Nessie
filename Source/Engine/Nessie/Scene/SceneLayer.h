// SceneLayer.h
#pragma once
#include "SceneNode.h"

#define NES_DEFINE_NODE_LAYER(layerTypename, nodeTypename)        \
NES_DEFINE_TYPE_INFO(layerTypename)                               \
public:                                                           \
    using NodeType = nodeTypename;                                \
private:

namespace YAML { class Node; }

namespace nes
{
    class Camera;
    class Event;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A SceneLayer manages a type of SceneNode in the Scene. For example,
    ///         the World is a SceneLayer that manages Actor Nodes, which exist in 3D space.
    //----------------------------------------------------------------------------------------------------
    class SceneLayer
    {
        friend class Scene;
        
    protected:
        Scene* m_pScene = nullptr;
        bool m_isBeingDestroyed = false;
        
    public:
        explicit SceneLayer(Scene* pScene) : m_pScene(pScene) {}
        virtual ~SceneLayer() = default;
        SceneLayer(const SceneLayer&) = delete;
        SceneLayer& operator=(const SceneLayer&) = delete;
        SceneLayer(SceneLayer&&) noexcept = delete;
        SceneLayer& operator=(SceneLayer&&) noexcept = delete;

        virtual void DestroyNode(const LayerHandle& handle) = 0;

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
        virtual void EditorRenderNodeHierarchy() = 0;
    };

    template <typename Type>
    concept NodeLayerType = !std::is_abstract_v<Type> && requires(Type layer)
    {
        TypeIsDerivedFrom<Type, SceneLayer>;
        ValidNodeType<typename Type::NodeType>;
    };
}
