// CollidePointResult.h
#pragma once
#include "Nessie/Physics/Body/BodyID.h"
#include "Shapes/SubShapeID.h"

namespace nes
{
    struct CollidePointResult
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Function required by the CollisionCollector. A smaller fraction is considered to be a 'better hit'.
        ///     For point queries there is no sensible return value. 
        //----------------------------------------------------------------------------------------------------
        inline float GetEarlyOutFraction() const { return 0.f; }

        BodyID m_bodyID;            /// Body that was hit.
        SubShapeID m_subShapeID;    /// Sub shape ID that we collided against.
    };
}
