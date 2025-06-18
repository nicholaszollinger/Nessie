// MulticastDelegate.h
#pragma once
#include <functional>
#include "Debug/Log.h"

// [TODO]: This is very unsafe at the minute with the dangling pointers.
// I would prefer a StrongPtr to a base object class, similar to Unreal. 

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Multicast delegate maintains a list of listeners that will be notified once Broadcast is
    ///     called.
    //----------------------------------------------------------------------------------------------------
    template <typename ... Args>
    class MulticastDelegate
    {
    public:
        using Callback = std::function<void(Args...)>;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a Listener to this event.
        //----------------------------------------------------------------------------------------------------
        void AddListener(void* pOwner, const Callback& callback);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a listener.
        //----------------------------------------------------------------------------------------------------
        void RemoveListener(void* pOwner);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Broadcast the event to all registered listeners.
        //----------------------------------------------------------------------------------------------------
        void Broadcast(Args... args);

    private:
        /// Map of listeners to this event.
        std::unordered_map<void*, Callback> m_listeners;
    };
    
    template <typename ... Args>
    void MulticastDelegate<Args...>::AddListener(void* pOwner, const Callback& callback)
    {
        if (m_listeners.contains(pOwner))
        {
            NES_WARN("MulticastDelegate: Attempted to add second callback to Delegate of the same owner.");
            return;
        }
        
        m_listeners.emplace(pOwner, callback);
    }

    template <typename ... Args>
    void MulticastDelegate<Args...>::RemoveListener(void* pOwner)
    {
        if (auto result = m_listeners.find(pOwner); result != m_listeners.end())
        {
            m_listeners.erase(result);
        }

        else
        {
            NES_WARN("MulticastDelegate: Attempted to remove listener from Delegate that doesn't exist.");
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Broadcast the event with the passed in the Arguments.
    //----------------------------------------------------------------------------------------------------
    template <typename ... Args>
    void MulticastDelegate<Args...>::Broadcast(Args... args)
    {
        for (auto& [pOwner, callback] : m_listeners)
        {
            callback(args...);
        }
    }
}
