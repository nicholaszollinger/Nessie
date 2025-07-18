// RenderCommandBuffer.h
#pragma once
#include "Nessie/Core/Memory/StrongPtr.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Under Development. Generic Command Buffer object.
    //----------------------------------------------------------------------------------------------------
    class RenderCommandBuffer : public RefTarget<RenderCommandBuffer>
    {
    public:
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Submit() = 0;

        static StrongPtr<RenderCommandBuffer> Create(uint32 count = 0, const std::string& debugName = "");
    };
}
