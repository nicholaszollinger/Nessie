// TickManager.h
#pragma once
#include "TickGroup.h"

namespace nes
{
    class TickManager
    {
        friend class SceneManager;
        ETickStage m_currentStage = ETickStage::PrePhysics;
        std::vector<std::vector<TickGroup*>> m_tickStageGroups{};
        double m_realTimeElapsed = 0.f;
        
    private:
        TickManager();
        ~TickManager();
    
    public:
        TickManager(const TickManager&) = delete;
        TickManager(TickManager&&) = delete;
        TickManager& operator=(const TickManager&) = delete;
        TickManager& operator=(TickManager&&) = delete;

        static TickManager& Get();
        
        void RegisterTickGroup(TickGroup* pGroup);
        void UnregisterTickGroup(TickGroup* pGroup);
        bool HasTickGroup(const TickGroup* pGroup) const;

        ETickStage GetCurrentTickStage() const { return m_currentStage; }
        
    private:
        bool Init();
        void Shutdown();
        void BeginFrame(const double deltaTime);
        void EndFrame();
        void RunTickStage(const ETickStage stage);
    };
}
