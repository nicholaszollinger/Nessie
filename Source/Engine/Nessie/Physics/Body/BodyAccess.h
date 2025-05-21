// BodyAccess.h
#pragma once
#include <cstdint>
#include "Debug/Assert.h"

namespace nes
{
    struct BodyAccess
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Access rules, used to detect race conditions during Physics simulation.  
        //----------------------------------------------------------------------------------------------------
        enum class Access : uint8_t
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
            inline GrantScope(Access velocity, Access position)
            {
                Access& currentVel = GetVelocityAccess();
                Access& currentPos = GetPositionAccess();

                NES_ASSERT(currentVel == Access::ReadWrite);
                NES_ASSERT(currentPos == Access::ReadWrite);

                currentVel = velocity;
                currentPos = position;
            }

            inline ~GrantScope()
            {
                GetVelocityAccess() = Access::ReadWrite;
                GetPositionAccess() = Access::ReadWrite;
            }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if we have certain permissions. 
        //----------------------------------------------------------------------------------------------------
        static inline bool CheckRights(const Access currentRights, const Access desiredRights)
        {
            return (static_cast<uint8_t>(currentRights) & static_cast<uint8_t>(desiredRights)) == static_cast<uint8_t>(desiredRights);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current thread's velocity access. 
        //----------------------------------------------------------------------------------------------------
        static inline Access& GetVelocityAccess()
        {
            static thread_local Access s_access = Access::ReadWrite;
            return s_access;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current thread's position access. 
        //----------------------------------------------------------------------------------------------------
        static inline Access& GetPositionAccess()
        {
            static thread_local Access s_access = Access::ReadWrite;
            return s_access;
        }
    };
}
