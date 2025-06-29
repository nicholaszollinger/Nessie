// SortReverseAndStore.h
#pragma once
#include "Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : This function will sort values from high to low and only keep the ones that are less than 'maxValue'.
    ///	@param inValues : Values to be sorted.
    ///	@param maxValue : Values need to be less than this to keep them.
    ///	@param identifiers : 4 identifiers that will be sorted in the same way as the values.
    ///	@param pOutValues : The values are sorted here from high to low.
    ///	@returns : The number of values that were kept.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE int SortReverseAndStore(const Vec4Reg inValues, const float maxValue, UVec4Reg& identifiers, float* pOutValues)
    {
        // Sort so that the highest values are first (we want to process the closer hits, and we process the stack from top to bottom). 
        Vec4Reg values = inValues;
        Vec4Reg::Sort4Reverse(values, identifiers);

        // Count how many results are less than the max value.
        const UVec4Reg closer = Vec4Reg::Less(values, Vec4Reg::Replicate(maxValue));
        const int numResults = closer.CountTrues();

        // Shift the values so that only the ones that are less than the max are kept.
        values = values.ReinterpretAsInt().ShiftComponents4Minus(numResults).ReinterpretAsFloat();
        identifiers = identifiers.ShiftComponents4Minus(numResults);

        // Store the values
        values.StoreFloat4(reinterpret_cast<Float4*>(pOutValues));

        return numResults;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Shift the elements so that the identifiers that correspond with the trues in 'value' come first.
    ///	@param value : Values to test for true or false.
    ///	@param identifiers : The identifiers that are shifted, on return they are shifted.
    ///	@returns : The number of trues.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE int CountAndSortTrues(const UVec4Reg& value, UVec4Reg& identifiers)
    {
        // Sort the hits
        identifiers = UVec4Reg::Sort4True(value, identifiers);

        // Return the number of hits.
        return value.CountTrues();
    }
}