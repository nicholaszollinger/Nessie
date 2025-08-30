// BodyAccess.h
#pragma once
#include <cstdint>
#include "Nessie/Debug/Assert.h"

namespace nes
{
    struct BodyAccess
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Access rules, used to detect race conditions during Physics simulation.  
        //----------------------------------------------------------------------------------------------------
        enum class EAccess : uint8_t
        {
            None        = 0,
            Read        = 1,
            ReadWrite   = 3,
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Grant scope specific access rights on the current thread. 
        //----------------------------------------------------------------------------------------------------
        struct GrantScope
        {
            inline GrantScope(const EAccess velocity, const EAccess position)
            {
                EAccess& currentVel = GetVelocityAccess();
                EAccess& currentPos = GetPositionAccess();

                NES_ASSERT(currentVel == EAccess::ReadWrite);
                NES_ASSERT(currentPos == EAccess::ReadWrite);

                currentVel = velocity;
                currentPos = position;
            }

            inline ~GrantScope()
            {
                GetVelocityAccess() = EAccess::ReadWrite;
                GetPositionAccess() = EAccess::ReadWrite;
            }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if we have certain permissions. 
        //----------------------------------------------------------------------------------------------------
        static inline bool CheckRights(const EAccess currentRights, const EAccess desiredRights)
        {
            return (static_cast<uint8_t>(currentRights) & static_cast<uint8_t>(desiredRights)) == static_cast<uint8_t>(desiredRights);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current thread's velocity access. 
        //----------------------------------------------------------------------------------------------------
        static inline EAccess& GetVelocityAccess()
        {
            static thread_local EAccess s_access = EAccess::ReadWrite;
            return s_access;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current thread's position access. 
        //----------------------------------------------------------------------------------------------------
        static inline EAccess& GetPositionAccess()
        {
            static thread_local EAccess s_access = EAccess::ReadWrite;
            return s_access;
        }
    };
}
