// BroadPhaseQuadTree.h
#pragma once
#include "BroadPhase.h"

namespace nes
{
    class BroadPhaseQuadTree final : public BroadPhase
    {
    public:
        virtual ~BroadPhaseQuadTree() override;

        //virtual void Init() override;

        virtual void Optimize() override;
        virtual void FrameSync() override;
        
    };
}
