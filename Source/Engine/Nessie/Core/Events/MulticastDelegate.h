// MulticastDelegate.h
#pragma once
#include <functional>
#include "Core/Log/Log.h"

// [TODO]: This is very unsafe at the minute with the dangling pointers.
// I would prefer a StrongPtr to an base object class, similar to unreal. 

namespace nes
{
    template <typename ... Args>
    class MulticastDelegate
    {
    public:
        using Callback = std::function<void(Args...)>;

    private:
        std::unordered_map<void*, Callback> m_listeners;

    public:
        void AddListener(void* pOwner, const Callback& callback);
        void RemoveListener(void* pOwner);
        void Broadcast(Args... args);
    };
    
    template <typename ... Args>
    void MulticastDelegate<Args...>::AddListener(void* pOwner, const Callback& callback)
    {
        if (m_listeners.contains(pOwner))
        {
            NES_WARNV("MulticastDelegate", "Attempted to add second callback to Delegate of the same owner.");
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
            NES_WARNV("MulticastDelegate", "Attempted to remove listener from Delegate that doesn't exist.");
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
