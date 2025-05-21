// CastResult.h
#pragma once
#include "Math/Generic.h"
#include "Physics/Body/BodyID.h"
#include "Shapes/SubShapeID.h"

namespace nes
{
    struct BroadPhaseCastResult
    {
        BodyID  m_bodyID;                                    /// Body that was hit.
        float   m_fraction = 1.f + math::PrecisionDelta();   /// Hit fraction fo the ray/object [0, 1]. HitPoint = Start + m_fraction * (End - Start). 

        
        //----------------------------------------------------------------------------------------------------
        /// @breif : Function required by the CollisionCollector. A smaller fraction is considered to be a 'better hit'.
        ///     For rays/cast shapes we can just use the collision fraction.
        //----------------------------------------------------------------------------------------------------
        inline float    GetEarlyOutFraction() const { return m_fraction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset this result so that is can be reused for a new cast. 
        //----------------------------------------------------------------------------------------------------
        inline void     Reset()
        {
            m_bodyID = BodyID();
            m_fraction = 1.f + math::PrecisionDelta();
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Specialization of the cast result against a shape. 
    //----------------------------------------------------------------------------------------------------
    struct RayCastResult : public BroadPhaseCastResult
    {
        SubShapeID m_subShapeID2; /// Sub shape ID of the shape that we collided against.
    };
}
