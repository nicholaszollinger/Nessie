// BinaryHeap.h
#pragma once
#include <iterator>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  See: https://en.wikipedia.org/wiki/Binary_heap
    //		
    /// @brief : Push a new element into a binary max-heap.
    ///     [begin, end - 1) must be a valid heap. Element end - 1 will be inserted into the heap.
    ///     The heap will be [begin, end) after this call.
    ///	@param begin : Start of the heap.
    ///	@param end : End of the heap.
    ///	@param predicate : A function that returns true if the first element is less or equal than the second element.
    //----------------------------------------------------------------------------------------------------
    template <typename Iterator, typename Predicate>
    void BinaryHeapPush(Iterator begin, Iterator end, Predicate predicate)
    {
        using diff_t = typename std::iterator_traits<Iterator>::difference_type;
        using elem_t = typename std::iterator_traits<Iterator>::value_type;

        // New heap size
        const diff_t count = std::distance(begin, end);

        // Start from the last element
        diff_t current = count - 1;
        while (current > 0)
        {
            // Get the current element
            elem_t& currentElem = *(begin + current);

            // Get the parent element
            diff_t parent = (current - 1) >> 1;
            elem_t& parentElem = *(begin + parent);

            // Sort them so that the parent is larger than the child
            if (predicate(parentElem, currentElem))
            {
                std::swap(parentElem, currentElem);
                current = parent;
            }
            else
            {
                // No change needed, we're done.
                break;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  See: https://en.wikipedia.org/wiki/Binary_heap
    //		
    /// @brief : Pop an element from a binary max-heap.
    ///     [begin, end - 1) must be a valid heap. The largest element will be removed from the heap.
    ///     The heap will be [begin, end - 1) after this call.
    ///	@param begin : Start of the heap.
    ///	@param end : End of the heap.
    ///	@param predicate : A function that returns true if the first element is less or equal than the second element.
    //----------------------------------------------------------------------------------------------------
    template <typename Iterator, typename Predicate>
    void BinaryHeapPop(Iterator begin, Iterator end, Predicate predicate)
    {
        using diff_t = typename std::iterator_traits<Iterator>::difference_type;

        // Begin by moving the highest element to the end, this is the popped element
        std::swap(*(end - 1), *begin);

        // New heap size
        diff_t count = std::distance(begin, end) - 1;

        // Start form the root
        diff_t largest = 0;
        for (;;)
        {
            // Get the first child
            diff_t child = (largest << 1) + 1;

            // Check if we're beyond the end of the heap. If so, the 2nd child will also be beyond the end.
            if (child >= count)
                break;

            // Remember the largest element from the previous iteration.
            diff_t prevLargest = largest;

            // Check if the first child is bigger, if so select it.
            if (predicate(*(begin + largest), *(begin + child)))
                largest = child;

            // Move to the next child.
            ++child;

            // Check if the second child is bigger, if so select it
            if (child < count && predicate(*(begin + largest), *(begin + child)))
                largest = child;

            // If there was no change, we're done.
            if (prevLargest == largest)
                break;

            // Swap the element
            std::swap(*(begin + prevLargest), *(begin + largest));
        }
    }
}
