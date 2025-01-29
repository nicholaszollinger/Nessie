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

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Define the global type alias (not in the nes namespace) for a mathematical type with the
///          NES_MATH_DEFAULT_REAL_TYPE precision.
///          For example, if the default real type is set to float, then writing
///          <code> NES_MATH_DEFINE_GLOBAL_TYPE_ALIAS_F(TRay2, Ray2D) </code>
///          declares Ray2D as an alias for nes::TRay2<float>.
///		@param type : Template mathematical type. Example: "TRay2" 
///		@param desiredTypename : The typename you want to give. Example "Ray2D".  
//----------------------------------------------------------------------------------------------------
#define NES_MATH_DECLARE_GLOBAL_TYPE_ALIAS_F(type, desiredTypename) \
using desiredTypename = nes::type<NES_MATH_DEFAULT_REAL_TYPE>;

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      This was made to enforce the naming convention for math template types: templated types must
//      prefix T, and I wanted to make the suffixes are all consistent.
//		
///		@brief : Defines floating-point type aliases for the templated mathematical type.
///             - Example usage for "TRay2": <code> NES_MATH_DECLARE_REAL_ALIASES_FOR_TEMPLATE(Ray2) </code>
///             - Notice that there is no "T" prefix in the parameter name! 
///             - This will define the aliases "Ray2f" for float and "Ray2d" for double.              
///		@param type : The template's typename, without the "T" prefix.
//----------------------------------------------------------------------------------------------------
#define NES_MATH_DECLARE_ALIASES_FOR_TEMPLATE_F(type)                   \
namespace nes                                                              \
{                                                                          \
    using type##f = T##type<float>;                                        \
    using type##d = T##type<double>;                                       \
}                                                                          \

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Defines integral type aliases for the templated mathematical type.
///             - Example usage for "TVector2": <code> NES_MATH_DECLARE_REAL_ALIASES_FOR_TEMPLATE(Vector2) </code>
///             - Notice that there is no "T" prefix in the parameter name! 
///             - This will define the aliases "Vector2i" for int and "Vector2u" for unsigned int.  
///		@param type : 
//----------------------------------------------------------------------------------------------------
#define NES_MATH_DECLARE_ALIASES_FOR_TEMPLATE_I(type)               \
namespace nes                                                              \
{                                                                          \
    using type##i = T##type<int>;                                          \
    using type##u = T##type<unsigned int>;                                 \
}