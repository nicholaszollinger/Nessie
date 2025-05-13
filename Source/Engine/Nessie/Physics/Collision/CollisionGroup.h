// CollisionGroup.h
#pragma once
#include <cstdint>

namespace nes
{
    class GroupFilter;

    //----------------------------------------------------------------------------------------------------
    //	TODO:
    //  I need to look at this class in addition to the CollisionLayer values to see if I want to change
    //  this system. Seems like a lot of the same kind of mechanisms/data.
    //
    //  This class seems to be the "serializable" data for objects. 
    //
    /// @brief :  
    //----------------------------------------------------------------------------------------------------
    class CollisionGroup
    {
    public:
        using GroupID       = uint32_t;
        using SubGroupID    = uint32_t;

        static constexpr GroupID    kInvalidGroup       = ~static_cast<GroupID>(0);
        static constexpr SubGroupID kInvalidSubGroup    = ~static_cast<SubGroupID>(0);

    private:
        const GroupFilter*          m_pFilter = nullptr;
        GroupID                     m_groupID = kInvalidGroup;
        SubGroupID                  m_subGroupID = kInvalidSubGroup;

    public:
        constexpr CollisionGroup() = default;
        CollisionGroup(const GroupFilter* pFilter, const GroupID groupID, const SubGroupID subgroupID) : m_pFilter(pFilter), m_groupID(groupID), m_subGroupID(subgroupID) {}

        void                SetGroupFilter(const GroupFilter* pFilter)  { m_pFilter = pFilter; }
        void                SetGroupID(const GroupID groupID)           { m_groupID = groupID; }
        void                SetSubGroupID(const SubGroupID subgroupID)  { m_subGroupID = subgroupID; }
        
        const GroupFilter*  GetGroupFilter() const                      { return m_pFilter; }
        GroupID             GetGroupID() const                          { return m_groupID; }
        SubGroupID          GetSubGroupID() const                       { return m_subGroupID; }

        // [TODO]: 
        bool                CanCollide(const CollisionGroup& other) const;
        
        static constexpr CollisionGroup Invalid() { return {}; }
    };
}
