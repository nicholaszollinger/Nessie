// TickManager.cpp
#include "TickManager.h"

#include "Debug/Assert.h"

namespace nes
{
    static TickManager* g_instance = nullptr;

    TickManager::TickManager()
    {
        NES_ASSERT(g_instance == nullptr);
        g_instance = this;

        m_tickStageGroups.resize(static_cast<uint8_t>(TickStage::NumStages));
    }

    TickManager::~TickManager()
    {
        g_instance = nullptr;
    }

    TickManager& TickManager::Get()
    {
        NES_ASSERT(g_instance != nullptr);
        return *g_instance;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //
    ///     @brief : 
    //----------------------------------------------------------------------------------------------------
    void TickManager::RegisterTickGroup(TickGroup* pGroup)
    {
        NES_ASSERT(pGroup != nullptr && !pGroup->m_isRegistered);
        NES_ASSERT(pGroup->GetStage() != TickStage::NumStages);
        
        const uint8_t stageValue = static_cast<uint8_t>(pGroup->GetStage());
        m_tickStageGroups[stageValue].push_back(pGroup);
        pGroup->m_isRegistered = true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //
    ///     @brief : 
    //----------------------------------------------------------------------------------------------------
    void TickManager::UnregisterTickGroup(TickGroup* pGroup)
    {
        NES_ASSERT(pGroup != nullptr && pGroup->m_isRegistered);
        NES_ASSERT(pGroup->GetStage() != TickStage::NumStages);

        const uint8_t stageValue = static_cast<uint8_t>(pGroup->GetStage());
        auto& groups = m_tickStageGroups[stageValue];

        for (size_t i = 0; i < groups.size(); i++)
        {
            if (groups[i] == pGroup)
            {
                pGroup->m_isRegistered = false;
                std::swap(groups[i], groups.back());
                groups.pop_back();
                break;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns whether this TickGroup is 
    //----------------------------------------------------------------------------------------------------
    bool TickManager::HasTickGroup(const TickGroup* pGroup) const
    {
        const uint8_t stageValue = static_cast<uint8_t>(pGroup->GetStage());
        auto& groups = m_tickStageGroups[stageValue];

        for (auto* pTickGroup : groups)
        {
            if (pTickGroup == pGroup)
            {
                NES_ASSERT(pTickGroup->m_isRegistered);
                return true;
            }
        }

        return false;
    }


    bool TickManager::Init()
    {
        // [TODO]: 
        return true;
    }

    void TickManager::Shutdown()
    {
        // [TODO]:
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Begin the frame for each Tick Group
    //----------------------------------------------------------------------------------------------------
    void TickManager::BeginFrame(const double deltaTime)
    {
        m_realTimeElapsed += deltaTime;
        
        for (auto& stageGroups : m_tickStageGroups)
        {
            for (auto& pGroup : stageGroups)
            {
                pGroup->BeginFrame(static_cast<float>(deltaTime), static_cast<float>(m_realTimeElapsed));
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief :  
    //----------------------------------------------------------------------------------------------------
    void TickManager::EndFrame()
    {
        
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Runs all Ticks in each group registered to a given stage. 
    //----------------------------------------------------------------------------------------------------
    void TickManager::RunTickStage(const TickStage stage)
    {
        NES_ASSERT(static_cast<uint8_t>(stage) < static_cast<uint8_t>(TickStage::NumStages));
        
        auto& stageGroups = m_tickStageGroups[static_cast<uint8_t>(stage)];
        for (auto& pGroup : stageGroups)
        {
            pGroup->ExecuteReadyTicks();
        }
    }
}
