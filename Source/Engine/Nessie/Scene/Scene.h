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

        // TimeInfo:
        double m_realTimeElapsed = 0.0f;                // The amount of Time elapsed since the start of the Application.
        double m_timeLeftForFixed = 0.f;                // The amount of Time left before the next Fixed Update is run.
        float m_fixedTimeStep = kDefaultFixedTimeStep;  // The Interval at which the Fixed Update is run.
        float m_worldTimeScale = 1.f;                   // Current Time Scale applied to the World.
        float m_sceneDeltaTime = 0.f;                   // Current Delta Time of the World, scaled by the World Time Scale.

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

        // Time:
        void SetGlobalTimeScale(const float timeScale);
        [[nodiscard]] float GetDeltaTime() const        { return m_sceneDeltaTime; }
        [[nodiscard]] float GetTimeScale() const        { return m_worldTimeScale; }
        [[nodiscard]] float GetFixedTimeStep() const    { return m_fixedTimeStep; }

    private:
        void PushLayer(const StrongPtr<EntityLayer>& pLayer);
        bool Init();
        bool Begin();
        void OnEvent(Event& event);
        void Tick(const double deltaRealTime);
        void PreRender();
        void Render();
        bool UpdateTime(const double deltaRealTime);
        void Destroy();
        
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
