// SubShapeIDPair.h
#pragma once
#include "SubShapeID.h"
#include "Math/Math.h"
#include "Physics/Body/BodyID.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains a paris of bodies and their sub shape ID's. Can be used as a key in a map to find
    ///     a contact point.
    //----------------------------------------------------------------------------------------------------
    class SubShapeIDPair
    {
    public:
        SubShapeIDPair() = default;

        SubShapeIDPair(const BodyID& body1ID, const SubShapeID& subShapeID1, const BodyID& body2ID, const SubShapeID& subShapeID2)
            : m_body1ID(body1ID)
            , m_subShape1ID(subShapeID1)
            , m_body2ID(body2ID)
            , m_subShape2ID(subShapeID2)
        {
            //
        }

        inline bool         operator==(const SubShapeIDPair& other) const
        {
            return UVec4Reg::Load(reinterpret_cast<const uint32_t*>(this)) == UVec4Reg::Load(reinterpret_cast<const uint32_t*>(&other));
        }

        /// Less than operator is used to consistently order contact points for a deterministic simulation.
        inline bool         operator<(const SubShapeIDPair& other) const
        {
            if (m_body1ID != other.m_body1ID)
                return m_body1ID < other.m_body1ID;

            if (m_subShape1ID.GetValue() != other.m_subShape1ID.GetValue())
                return m_subShape1ID.GetValue() < other.m_subShape1ID.GetValue();

            if (m_body2ID != other.m_body2ID)
                return m_body2ID < other.m_body2ID;

            return m_subShape2ID.GetValue() < other.m_subShape2ID.GetValue();
        }

        const BodyID&       GetBody1ID() const          { return m_body1ID; }
        const BodyID&       GetBody2ID() const          { return m_body2ID; }
        const SubShapeID&   GetSubShape1ID() const      { return m_subShape1ID; }
        const SubShapeID&   GetSubShape2ID() const      { return m_subShape1ID; }

        // [TODO]: 
        uint64_t            GetHash() const;

    private:
        BodyID      m_body1ID;
        SubShapeID  m_subShape1ID;
        BodyID      m_body2ID;
        SubShapeID  m_subShape2ID;
    };

    static_assert(sizeof(SubShapeIDPair) == 16, "Unexpected size");
    static_assert(alignof(SubShapeIDPair) == 4, "Assuming 4 byte aligned");
}