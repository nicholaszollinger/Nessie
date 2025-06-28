// ScopeExit.h
#pragma once
#include "Config.h"
#include "Generic/Concepts.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that calls a function when it goes out of scope. 
    //----------------------------------------------------------------------------------------------------
    template <CallableType FuncType>
    class ScopeExit
    {
    public:
        /// Constructor specifies the exit function.
        NES_INLINE explicit ScopeExit(FuncType&& function) : m_function(std::move(function)) {}

        /// Destructor calls the exit function
        NES_INLINE          ~ScopeExit() { if (!m_invoked) m_function(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call the exit function now instead of when this falls out of scope. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void     Invoke()
        {
            if (!m_invoked)
            {
                m_function();
                m_invoked = true;
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : No longer call the exit function when going out of scope. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void     Release() { m_invoked = true; }
        
    private:
        FuncType            m_function;
        bool                m_invoked = false;
    };


    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper macros to give a unique name to the local ScopedExit object.
    ///     Do not call. Use NES_SCOPE_EXIT().
    //----------------------------------------------------------------------------------------------------
    #define NES_SCOPE_EXIT_TAG2(line)   scope_exit##line
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper macros to give a unique name to the local ScopedExit object.
    ///     Do not call. Use NES_SCOPE_EXIT().
    //----------------------------------------------------------------------------------------------------
    #define NES_SCOPE_EXIT_TAG(line)    NES_SCOPE_EXIT_TAG2(line)

    //----------------------------------------------------------------------------------------------------
    /// @brief : Usage: NES_SCOPE_EXIT([] { code to call on scope exit }); 
    //----------------------------------------------------------------------------------------------------
    #define NES_ON_SCOPE_EXIT(...) ScopeExit NES_SCOPE_EXIT_TAG(__LINE__)(__VA_ARGS__)
}

