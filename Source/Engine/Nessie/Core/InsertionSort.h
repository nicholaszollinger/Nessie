// InsertionSort.h
#pragma once
#include <functional>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation of the Insertion Sort algorithm. 
    //----------------------------------------------------------------------------------------------------
    template <typename IteratorType, typename CompareType>
    void InsertionSort(IteratorType begin, IteratorType end, CompareType compare = std::less<>{})
    {
        if (begin != end)
        {
            for (IteratorType i = begin + 1; i != end; ++i)
            {
                auto temp = std::move(*i);

                // Check if the element goes before begin (we can't decrement the iterator before begin so
                // this needs to be a separate branch).
                if (compare(temp, *begin))
                {
                    // Move all elements to the right to make space for temp
                    IteratorType previous;
                    for (IteratorType j = i; j != begin; j = previous)
                    {
                        previous = j - 1;
                        *j = *previous;
                    }

                    // Move x to the first place
                    *begin = std::move(temp);
                }
                else
                {
                    IteratorType j = i;
                    for (IteratorType previous = j - 1; compare(temp, *previous); j = previous, --previous)
                    {
                        *j = std::move(*previous);
                    }

                    // Move temp into place
                    *j = std::move(temp);
                }
            }
        }
    }
}
