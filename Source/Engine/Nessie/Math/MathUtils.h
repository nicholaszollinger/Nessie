// MathUtils.h
#pragma once

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      I made this so that I can keep the default precision of global aliased math types consistent.
//		
///		@brief : Either float or double. Used when defining the global type alias for a template math type.
///             For example, if this is set to float, then when using the macro
///             <code> NES_MATH_DEFINE_GLOBAL_TYPE_ALIAS_F(TRay2, Ray2D) </code>
///             declares Ray2D as an alias for nes::TRay2<float>.
//----------------------------------------------------------------------------------------------------
#define NES_MATH_DEFAULT_REAL_TYPE float