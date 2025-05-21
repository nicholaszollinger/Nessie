// AABoxSIMD.h
#pragma once
#include "Math/VectorRegister.h"
#include "Math/AABox.h"

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
    NES_INLINE VectorRegisterUint AABoxVs4AABox(const AABox& box, const VectorRegister& box4MinX, const VectorRegister& box4MinY, const VectorRegister& box4MinZ, const VectorRegister& box4MaxX, const VectorRegister& box4MaxY, const VectorRegister& box4MaxZ)
    {
        // Splat the values of the single box
        // (Replicate the specific component value among all components in the register).
        const VectorRegister boxMinX = VectorRegister::Replicate(box.m_min.x);
        const VectorRegister boxMinY = VectorRegister::Replicate(box.m_min.y);
        const VectorRegister boxMinZ = VectorRegister::Replicate(box.m_min.z);
        const VectorRegister boxMaxX = VectorRegister::Replicate(box.m_max.x);
        const VectorRegister boxMaxY = VectorRegister::Replicate(box.m_max.y);
        const VectorRegister boxMaxZ = VectorRegister::Replicate(box.m_max.z);

        // Test separation over each axis:
        VectorRegisterUint noOverlapX = VectorRegisterUint::Or(VectorRegister::Greater(boxMinX, box4MaxX), VectorRegister::Greater(box4MinX, boxMaxX));
        VectorRegisterUint noOverlapY = VectorRegisterUint::Or(VectorRegister::Greater(boxMinY, box4MaxY), VectorRegister::Greater(box4MinY, boxMaxY));
        VectorRegisterUint noOverlapZ = VectorRegisterUint::Or(VectorRegister::Greater(boxMinZ, box4MaxZ), VectorRegister::Greater(box4MinZ, boxMaxZ));

        // Return Overlap:
        return VectorRegisterUint::Not(VectorRegisterUint::Or(VectorRegisterUint::Or(noOverlapX, noOverlapY), noOverlapZ));
    }
}