// MoveElement.h
#pragma once
#include <algorithm>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Move an element to a specific position in the container, while preserving order.
    ///     Returns an iterator from to the moved element.
    ///	@tparam Container : std container with random access. 
    ///	@param container : Container object.
    ///	@param from : The position of the element you want to move.
    ///	@param to : The position in the container that you want to place the element.
    ///	@param insertAfter : Whether you want to place the element before or after the 'to' position.
    //----------------------------------------------------------------------------------------------------
    template <typename Container> requires std::ranges::random_access_range<Container>
    Container::iterator MoveElement(Container& container, typename Container::iterator from, typename Container::iterator to, const bool insertAfter = true)
    {
        if (container.empty() || from == to)
            return from;

        // Determine the destination index where the element should end up
        auto dest = insertAfter? std::next(to) : to;
        
        if (from == dest)
            return from;

        // Check if rotating forward or backward:
        if (from < dest)
            std::rotate(from, std::next(from), dest);
        else
            std::rotate(dest, from, std::next(from));

        // Compute the new iterator to the moved element
        // If we moved forward, it lands right before `dest`
        // If we moved backward, it now starts at `dest`
        return (from < dest) ? std::prev(dest) : dest;
    }
}