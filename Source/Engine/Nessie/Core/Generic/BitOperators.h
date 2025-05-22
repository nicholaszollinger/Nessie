// BitOperators.h
#pragma once
#include "Concepts.h"

//-----------------------------------------------------------------------------------------------------------------------------
///		@brief : Create the bitwise operators for an Enum, to be able to use the enum as a bitmask.
//-----------------------------------------------------------------------------------------------------------------------------
#define NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(Enum)                                                            \
    static_assert(nes::EnumType<Enum>, "Type must be an enum type!");                       \
    inline constexpr Enum operator|(const Enum left, const Enum right)                      \
    {                                                                                       \
        return static_cast<Enum>(                                                           \
            static_cast<std::underlying_type_t<Enum>>(left) |                               \
            static_cast<std::underlying_type_t<Enum>>(right));                              \
    }                                                                                       \
                                                                                            \
    inline constexpr Enum operator&(const Enum left, const Enum right)                      \
    {                                                                                       \
        return static_cast<Enum>(                                                           \
            static_cast<std::underlying_type_t<Enum>>(left) &                               \
            static_cast<std::underlying_type_t<Enum>>(right));                              \
    }                                                                                       \
                                                                                            \
    inline constexpr Enum operator^(const Enum left, const Enum right)                      \
    {                                                                                       \
        return static_cast<Enum>(                                                           \
            static_cast<std::underlying_type_t<Enum>>(left) ^                               \
            static_cast<std::underlying_type_t<Enum>>(right));                              \
    }                                                                                       \
                                                                                            \
    inline constexpr Enum operator~(const Enum value)                                       \
    {                                                                                       \
        return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(value));        \
    }                                                                                       \
                                                                                            \
    inline constexpr Enum operator|=(Enum& left, const Enum right)                          \
    {                                                                                       \
        return left = left | right;                                                         \
    }                                                                                       \
                                                                                            \
    inline constexpr Enum operator&=(Enum& left, const Enum right)                          \
    {                                                                                       \
        return left = left & right;                                                         \
    }                                                                                       \
                                                                                            \
    inline constexpr Enum operator^=(Enum& left, const Enum right)                          \
    {                                                                                       \
        return left = left ^ right;                                                         \
    }