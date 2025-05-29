// Scene.h
#pragma once
#include <filesystem>
#include <functional>
#include "EntityLayer.h"
#include "Core/Events/Event.h"
#include "Core/Memory/StrongPtr.h"
#include "Math/Matrix.h"

namespace nes
{
    class Camera;
    class WorldComponent;

    NES_DEFINE_LOG_TAG(kSceneLogTag, "Scene", Info);
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Scene manages a stack of NodeLayers and processed in the following order:
    ///         - Ticked from bottom to top.
    ///         - Renderer from bottom to top.
    ///         - Propagate Events from top to bottom.
    //----------------------------------------------------------------------------------------------------
    class Scene
    {
        static constexpr float kDefaultFixedTimeStep = (1.f / 60.f);

        friend class SceneManager;

        std::vector<StrongPtr<EntityLayer>>   m_layerStack{};
        const Camera*                       m_pActiveCamera = nullptr;
        StringID                            m_name;
        bool                                m_isBeingDestroyed = false;

    public:
        Scene() = default;
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) noexcept = delete;
        Scene& operator=(Scene&&) noexcept = delete;

        void SetActiveCamera(const Camera* camera);
        [[nodiscard]] const Camera* GetActiveCamera() const { return m_pActiveCamera; }

        template <EntityLayerType Type>
        StrongPtr<Type> GetLayer() const;
        
    private:
        void PushLayer(const StrongPtr<EntityLayer>& pLayer);
        bool Init();
        bool Begin();
        void OnEvent(Event& event);
        void PreRender();
        void Render();
        void Destroy();
        void OnPostTick();
        
        bool Load(const std::filesystem::path& scenePath);
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the first EntityLayer matching the Type. If no Layer exists, then this returns
    ///         nullptr.
    //----------------------------------------------------------------------------------------------------
    template <EntityLayerType Type>
    StrongPtr<Type> Scene::GetLayer() const
    {
        for (auto& layer : m_layerStack)
        {
            if (layer->GetTypeID() == Type::GetStaticTypeID())
            {
                return layer;
            }
        }

        return nullptr;
    }
}
