// GroupFilter.h
#pragma once
#include "Nessie/Core/Result.h"
#include "Nessie/Core/Memory/RefCounter.h"
#include "Nessie/Core/Memory/StrongPtr.h"

namespace nes
{
    class CollisionGroup;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Abstract class that determines if two collision groups collide. 
    //----------------------------------------------------------------------------------------------------
    class GroupFilter : public RefTarget<GroupFilter>
    {
    public:
        using GroupFilterResult = Result<StrongPtr<GroupFilter>>;
        virtual ~GroupFilter() override = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if two groups can collide.
        //----------------------------------------------------------------------------------------------------
        virtual bool CanCollide(const CollisionGroup& group1, const CollisionGroup& group2) const = 0;
    };
}
