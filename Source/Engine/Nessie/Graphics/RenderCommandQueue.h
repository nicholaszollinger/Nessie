// RenderCommandQueue.h
#pragma once
#include "Nessie/Core/Config.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Buffer of commands to be executed on the Renderer. The buffer is allocated once at 
    ///     construction. Used for Renderer::Submit() and Renderer::SubmitResourceFree().
    //----------------------------------------------------------------------------------------------------
    class RenderCommandQueue
    {
    public:
        /// Function pointer that takes in a functor to call. See Renderer::Submit() for usage.
        using RenderCommandFunc = void(*)(void*);

        RenderCommandQueue();
        ~RenderCommandQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate a new command for the Renderer to execute.
        ///	@param func : Function pointer wrapper that will receive the functor object.
        ///	@param size : Size of the functor object.
        ///	@returns : The address of the functor object. Use this to in-place new the functor object.
        //----------------------------------------------------------------------------------------------------
        void*   Allocate(RenderCommandFunc func, const uint32 size);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Execute all commands in the buffer. Resets the buffer.
        //----------------------------------------------------------------------------------------------------
        void    Execute();

    private:
        static constexpr size_t kBufferSize = static_cast<size_t>(10 * 1024 * 1024); // 10MB.
        
        uint8*  m_pCommandsBuffer;          /// The allocated array to contain commands.
        uint8*  m_pCurrentWritePosition;    /// Address in the commands buffer to allocate commands in.
        uint32  m_commandCount = 0;         /// The number of commands that have been allocated.
    };
}