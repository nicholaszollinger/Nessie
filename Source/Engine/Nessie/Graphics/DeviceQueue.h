// GQueue.h
#pragma once
#include "GraphicsCommon.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    class RenderDevice;

    class DeviceQueue
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin identifying a region of interest for queue commands. Bookend with EndAnnotation().
        ///	@param name : Name of the label. 
        ///	@param color : Color for the label. 
        //----------------------------------------------------------------------------------------------------
        void            BeginAnnotation(const char* name, LinearColor color = LinearColor::Zero());
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : End a labeled region started with BeginAnnotation().
        //----------------------------------------------------------------------------------------------------
        void            EndAnnotation();
    
        EGraphicsResult Submit(const QueueSubmitDesc& submitDesc, const GSwapchain* pSwapchain);
        
        EGraphicsResult WaitUntilIdle();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this Device Queue's type. 
        //----------------------------------------------------------------------------------------------------
        nri::EQueueType      GetType() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the mutex associated with this Queue. This is locked for Submit and WaitIdle commands.
        //----------------------------------------------------------------------------------------------------
        //Mutex&          GetMutex() { return m_mutex; }

    private:
        //Mutex       m_mutex;
        //EQueueType  m_type = EQueueType::Graphics;
    };
    
    // class GQueue // : public DebugNameBase
    // {
    // public:
    //     GQueue() = default;
    //     virtual ~GQueue() = default;
    //     GQueue(const GQueue&) = delete;
    //     GQueue(GQueue&&) noexcept = delete;
    //     GQueue& operator=(const GQueue&) = delete;
    //     GQueue& operator=(GQueue&&) noexcept = delete;
    //     
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Begin identifying a region of interest for queue commands. Bookend with EndAnnotation().
    //     ///	@param name : Name of the label. 
    //     ///	@param color : Color for the label. 
    //     //----------------------------------------------------------------------------------------------------
    //     virtual void        BeginAnnotation(const char* name, LinearColor color = LinearColor::Zero()) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : End a labeled region started with BeginAnnotation().
    //     //----------------------------------------------------------------------------------------------------
    //     virtual void        EndAnnotation() = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Get what type of queue this is. 
    //     //----------------------------------------------------------------------------------------------------
    //     inline EQueueType   GetType() const { return m_type; }
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Get the mutex associated with this Queue. This is locked for Submit and WaitIdle commands.
    //     //----------------------------------------------------------------------------------------------------
    //     inline Mutex&       GetMutex() { return m_mutex; }
    //
    //     virtual EGraphicsResult Submit(const QueueSubmitDesc& submitDesc, const Swapchain* pSwapchain) = 0;
    //
    //     virtual EGraphicsResult WaitUntilIdle() = 0;
    //
    // private:
    //     Mutex       m_mutex;
    //     EQueueType  m_type = EQueueType::Graphics;
    // };
}
