// AABoxSIMD.h
#pragma once
#include "Geometry/AABox.h"

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Test a single box against 4 boxes with dimensions split into registers.
    ///	@param box : Box to test against.
    ///	@param box4MinX : The min X values of the 4 boxes.
    ///	@param box4MinY : The min Y values of the 4 boxes. 
    ///	@param box4MinZ : The min Z values of the 4 boxes.
    ///	@param box4MaxX : The max X values of the 4 boxes.
    ///	@param box4MaxY : The max Y values of the 4 boxes.
    ///	@param box4MaxZ : The max Z values of the 4 boxes.
    ///	@returns : An integer register class where each component represents if the box collided or not.
    ///     If the X component is == 1, then the box represented as the first component of each of the
    ///     register inputs collides with the box.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE UVec4Reg AABoxVs4AABox(const AABox& box, const Vec4Reg& box4MinX, const Vec4Reg& box4MinY, const Vec4Reg& box4MinZ, const Vec4Reg& box4MaxX, const Vec4Reg& box4MaxY, const Vec4Reg& box4MaxZ)
    {
        // Splat the values of the single box
        // (Replicate the specific component value among all components in the register).
        const Vec4Reg boxMinX = Vec4Reg::Replicate(box.m_min.x);
        const Vec4Reg boxMinY = Vec4Reg::Replicate(box.m_min.y);
        const Vec4Reg boxMinZ = Vec4Reg::Replicate(box.m_min.z);
        const Vec4Reg boxMaxX = Vec4Reg::Replicate(box.m_max.x);
        const Vec4Reg boxMaxY = Vec4Reg::Replicate(box.m_max.y);
        const Vec4Reg boxMaxZ = Vec4Reg::Replicate(box.m_max.z);

        // Test separation over each axis:
        UVec4Reg noOverlapX = UVec4Reg::Or(Vec4Reg::Greater(boxMinX, box4MaxX), Vec4Reg::Greater(box4MinX, boxMaxX));
        UVec4Reg noOverlapY = UVec4Reg::Or(Vec4Reg::Greater(boxMinY, box4MaxY), Vec4Reg::Greater(box4MinY, boxMaxY));
        UVec4Reg noOverlapZ = UVec4Reg::Or(Vec4Reg::Greater(boxMinZ, box4MaxZ), Vec4Reg::Greater(box4MinZ, boxMaxZ));

        // Return Overlap:
        return UVec4Reg::Not(UVec4Reg::Or(UVec4Reg::Or(noOverlapX, noOverlapY), noOverlapZ));
    }
}