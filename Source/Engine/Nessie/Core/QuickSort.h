// QuickSort.h
#pragma once
#include "Nessie/Core/InsertionSort.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    namespace internal
    {
        //----------------------------------------------------------------------------------------------------
        //	Reference: https://en.wikipedia.org/wiki/Quicksort, using Hoare's partition scheme.
        //      See: Implementation Issues -> "Choice of Pivot"
        //
        /// @brief : Helper function for QuickSort(). This will move the pivot element to "middle".
        //----------------------------------------------------------------------------------------------------
        template <typename IteratorType, typename CompareType>
        void QuickSortMedianOfThree(IteratorType first, IteratorType middle, IteratorType last, CompareType compare)
        {
            NES_ASSERT(first != middle && middle != last);

            if (compare(*middle, *first))
                std::swap(*first, *middle);

            if (compare(*last, *first))
                std::swap(*first, *last);

            if (compare(*last, *middle))
                std::swap(*middle, *last);
        }

        //----------------------------------------------------------------------------------------------------
        //	Reference: https://en.wikipedia.org/wiki/Quicksort, using Hoare's partition scheme.
        //      See: Implementation Issues -> "Choice of Pivot"
        // 
        /// @brief : Helper function for QuickSort(). This uses the Ninther method, which will move the pivot
        ///     to the middle.
        //----------------------------------------------------------------------------------------------------
        template <typename IteratorType, typename CompareType>
        void QuickSortNinther(IteratorType first, IteratorType middle, IteratorType last, CompareType compare)
        {
            // Divide the range in 8 equal parts (this means there are 9 points).
            auto difference = (last - first) >> 3;
            auto twoDifference = difference << 2;

            // Median of the first 3 points:
            IteratorType middle1 = first + difference;
            QuickSortMedianOfThree(first, middle1, first + twoDifference, compare);

            // Median of hte second 3 points:
            QuickSortMedianOfThree(middle - difference, middle, middle + difference, compare);

            // Median of the third 3 points:
            IteratorType middle3 = last - difference;
            QuickSortMedianOfThree(last - twoDifference, middle3, last, compare);
            
            // Determine the Median of the 3 Medians.
            QuickSortMedianOfThree(middle1, middle, middle3, compare);
        }
    }

    //----------------------------------------------------------------------------------------------------
    //  According to the Jolt Physics codebase, the STL version is not consistent across platforms.
    //  Reference: https://en.wikipedia.org/wiki/Quicksort, using Hoare's partition scheme.
    //		
    /// @brief : Implementation of the Quicksort algorithm.
    /// @note : If the number of elements is <= 32, this will fall back to using InsertionSort. 
    //----------------------------------------------------------------------------------------------------
    template <typename IteratorType, typename CompareType>
    void QuickSort(IteratorType begin, IteratorType end, CompareType compare = std::less<>{})
    {
        // Loop so that we only need to do 1 recursive call instead of 2.
        for (;;)
        {
            auto numElements = end - begin;
            if (numElements < 2)
                return;
            
            // Fall back to insertion sort if there are too few elements.
            if (numElements <= 32)
            {
                InsertionSort(begin, end, compare);
                return;
            }

            // Determine Pivot
            IteratorType pivotIterator = begin + ((numElements - 1) >> 1);
            internal::QuickSortNinther(begin, pivotIterator, end - 1, compare);

            auto pivot = *pivotIterator;

            // Left and right iterators
            IteratorType left = begin;
            IteratorType right = end;

            for (;;)
            {
                // Find the first element that is bigger than the pivot
                while (compare(*left, pivot))
                    ++left;

                // Find the last element that is smaller than the pivot
                do
                {
                    --right;
                }
                while (compare(pivot, *right));

                // If the two iterators crossed, we're done.
                if (left >= right)
                    break;

                // Swap the elements
                std::swap(*left, *right);

                // Note that the first while loop in this function should
                // have been "do i++ while (...)" but since we cannot decrement
                // the iterator from begin we left that out, so we need to do it
                // here.
                ++left;
            }

            // Include the middle element on the left side.
            ++right;

            // Check which partition is smaller
            if (right - begin < end - right)
            {
                // Left side is smaller, recurse the left first
                QuickSort(begin, right, compare);

                // Loop again with the right side to avoid a call.
                begin = right;
            }
            else
            {
                // Right side is smaller, recurse to the right first
                QuickSort(right, end, compare);

                // Loop again with the left side to avoid a call.
                end = right;
            }
        }
    }
}