// Swapchain.h
#pragma once
#include "GraphicsCore.h"
#include "Nessie/Core/Memory/StrongPtr.h"

//-------------------------------------------------------------------------------------------------
// Under development. I am in the process of abstracting the Vulkan API.
//-------------------------------------------------------------------------------------------------

// /// Special "initialValue" for "CreateFence" needed to create swap chain related semaphores
// static constexpr uint64 kSwapchainSemaphore = static_cast<uint64>(-1);
//
// //----------------------------------------------------------------------------------------------------
// /// @brief : Swapchain-specific format types.
// // Color space:
// //  - BT.709 - LDR https://en.wikipedia.org/wiki/Rec._709
// //  - BT.2020 - HDR https://en.wikipedia.org/wiki/Rec._2020
// // Transfer function:
// //  - G10 - linear (gamma 1.0)
// //  - G22 - sRGB (gamma ~2.2)
// //  - G2084 - SMPTE ST.2084 (Perceptual Quantization)
// // Bits per channel:
// //  - 8, 10, 16 (float) 
// //----------------------------------------------------------------------------------------------------
// enum class ESwapchainFormat : uint8
// {
//     BT709_G10_16BIT,    // LDR, Linear, 16 bit.
//     BT709_G22_8BIT,     // LDR, sRGB, 8 bit.
//     BT709_G22_10BIT,    // LDR, sRGB, 10 bit.
//     BT2020_G2084_10BIT  // HDR, SMPTE, 10 bit.
// };
//

// struct ChromaticityCoords
// {
//     float x; // [0, 1]
//     float y; // [0, 1]
// };
//
// struct DisplayDesc
// {
//     ChromaticityCoords  m_redPrimary;
//     ChromaticityCoords  m_greenPrimary;
//     ChromaticityCoords  m_bluePrimary;
//     ChromaticityCoords  m_whitePoint;
//     float               m_minLuminance;
//     float               m_maxLuminance;
//     float               m_maxFullFrameLuminance;
//     bool                m_isHDR;
// };

// struct SwapchainInterface
// {
//     EGraphicsErrorCodes             (*CreateSwapchain)(Device& device, const SwapchainDesc& swapchainDesc, Swapchain*& pOutSwapchain);
//     void                (*DestroySwapchain)(Swapchain& swapchain);
//     Texture* const*     (*GetSwapChainTextures)(const Swapchain& swapchain, uint32& outNumTextures);
//
//     //----------------------------------------------------------------------------------------------------
//     /// @brief : Get the display description info for the current display.
//     /// Returns EResult::Failure if the swapchain's window is outside all the monitors.
//     //----------------------------------------------------------------------------------------------------
//     EGraphicsErrorCodes             (*GetDisplayDesc)(Swapchain& swapchain, DisplayDesc& outDisplayDesc);
//
//     //----------------------------------------------------------------------------------------------------
//     /// @brief : Get the next texture to render into.
//     /// Vulkan: may return "OutOfDate"; fences must be created with "kSwapchainSemaphore" initial value. 
//     //----------------------------------------------------------------------------------------------------
//     EGraphicsErrorCodes             (*AcquireNextTexture)(Swapchain& swapchain, Fence& acquireSemaphore, uint32& outTextureIndex);
//
//     //----------------------------------------------------------------------------------------------------
//     /// @brief : Wait for the swapchain to finish presenting to the output texture.
//     /// Call once right before input sampling (must be called starting from the 1st frame)
//     //----------------------------------------------------------------------------------------------------
//     EGraphicsErrorCodes             (*WaitForPresent)(Swapchain& swapchain);
//     EGraphicsErrorCodes             (*QueuePresent) (Swapchain& swapchain, Fence& releaseSemaphore);
// };

namespace nes
{
    class Swapchain;
    class ApplicationWindow;

    enum class ESwapchainBits : uint8
    {
        None            = 0,
        Vsync           = NES_BIT(0),   // Cap framerate to the monitor refresh rate.
        Waitable        = NES_BIT(1),   // Unlock "WaitForPresent" reducing latency (requires m_features.m_waitableSwapchain").
        AllowTearing    = NES_BIT(2),   // Allow screen tearing if possible.
        AllowLowLatency = NES_BIT(3),   // Allow "LowLatency" functionality (requires "features.m_lowLatency").
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(ESwapchainBits);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Expects NES_PLATFORM_WINDOWS macro. 
    //----------------------------------------------------------------------------------------------------
    struct WindowsWindow
    {
        void*           m_hwnd;
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Add other platforms as necessary. 
    /// @brief : Group of native window handles for different platforms. Only one will be valid, depending
    ///     on the current platform.
    //----------------------------------------------------------------------------------------------------
    struct NativeWindow
    {
        WindowsWindow   m_windows;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Info for creating a swapchain.
    ///  Swapchain textures will be created as "color attachment" resources.
    ///  - m_numFramesInFlight = 0 - auto-selection between 1 (for waitable) or 2 (otherwise)
    ///  - m_numFramesInFlight = 2 - recommended if the GPU frame time is less than the desired frame time, but the sum of 2 frames is greater
    //----------------------------------------------------------------------------------------------------
    struct SwapchainDesc
    {
        ApplicationWindow*  m_pWindow = nullptr;
        const DeviceQueue*  m_pQueue;               /// Graphics or Compute (requires "features.m_presentFromCompute").
        DimType             m_width;
        DimType             m_height;
        uint8               m_numTextures;          /// Desired value. The real value must be queried using "GetSwapchainTextures"
        uint8               m_numFramesInFlight = 0; 
    };

    struct SwapchainTexture
    {
        // [TODO]: These should all be 
        GFence*     m_pAcquireSemaphore;
        GFence*     m_pReleaseSemaphore;
        GTexture*   m_pTexture;
        Descriptor* m_pColorAttachment;
        //EFormat     m_attachmentFormat;
    };

    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        // No copy or move.
        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&&) noexcept = delete;
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the swapchain resources. Returning false is considered a fatal error. 
        //----------------------------------------------------------------------------------------------------
        virtual bool    Create(uint32 width, uint32 height) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy the swapchain resources. 
        //----------------------------------------------------------------------------------------------------
        virtual void    Destroy() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle resize events from the window.
        ///	@param width : Window width.
        ///	@param height : Window height.
        ///	@param vsyncEnabled : Whether the window has Vsync enabled or not.
        //----------------------------------------------------------------------------------------------------
        virtual void    OnResize(uint32 width, uint32 height, const bool vsyncEnabled) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resize using the current set width and height. 
        //----------------------------------------------------------------------------------------------------
        virtual void    Resize() = 0;
    
        virtual bool    BeginFrame() = 0;
        virtual void    Present() = 0;

        uint32          GetCurrentBackBufferIndex();

    protected:
        //inline uint8    GetNumQueuedFrames() const              { return IsVsyncEnabled()? 2 : 3; }
        //inline uint8    GetOptimalNumSwapchainTextures() const  { return GetNumQueuedFrames() + 1; }
        //bool            IsVsyncEnabled() const;
    
    };
}