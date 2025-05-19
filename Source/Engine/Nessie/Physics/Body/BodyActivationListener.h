// BodyActivationListener.h
#pragma once
#include <cstdint>

namespace nes
{
    class BodyID;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for the Listener that will be notified whenever a body is activated or deactivated.
    //----------------------------------------------------------------------------------------------------
    class BodyActivationListener
    {
    public:
        virtual ~BodyActivationListener() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called whenever a body activates. At the time of the callback the body "bodyID" will be locked
        ///     and no bodies can be written/activated/deactivated from the callback.
        /// @note : This can be called from any thread so make sure that the code is thread safe!
        //----------------------------------------------------------------------------------------------------
        virtual void OnBodyActivated(const BodyID& bodyID, uint64_t bodyUserData) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called whenever a body deactivates. At the time of the callback the body "bodyID" will be locked
        ///     and no bodies can be written/activated/deactivated from the callback.
        /// @note : This can be called from any thread so make sure that the code is thread safe!
        //----------------------------------------------------------------------------------------------------
        virtual void OnBodyDeactivated(const BodyID& bodyID, uint64_t bodyUserData) = 0;
    };
}