#pragma once
// Concepts.hpp
#include <array>
#include <concepts>
#include <vector>

namespace nes
{
    template <typename Type>
    concept FloatingPointType = std::floating_point<Type>;

    template <typename Type>
    concept IntergralType = std::integral<Type>;
    
    template <typename Type>
    concept ScalarType = std::floating_point<Type> || std::integral<Type>;

    template <typename Type>
    concept UnsignedIntegralType = std::is_unsigned_v<Type>;

    template <typename Type>
    concept SignedIntergralType = std::integral<Type> && !std::is_unsigned_v<Type>;

    template <typename Type>
    concept PointerType = std::is_pointer_v<Type>;

    template <typename Type>
    concept ReferenceType = std::is_reference_v<Type>;

    template <typename Type>
    concept PointerOrReferenceType = PointerType<Type> || ReferenceType<Type>;

    template <typename Type>
    concept EnumType = std::is_enum_v<Type>;

    template <typename Type, typename...ConstructorParams>
    concept ValidConstructorForType = requires(ConstructorParams&&...params)
    {
        Type(params...);
    };

    template <typename Type, typename OtherType>
    concept IsBaseOrDerived = (std::is_base_of_v<Type, OtherType> || std::is_base_of_v<OtherType, Type>);

    template <typename Type>
    concept ArrayOrVector = std::same_as<Type, std::array<typename Type::value_type, Type::size()>> || std::same_as<Type, std::vector<typename Type::value_type>>
                            || std::same_as<Type, std::vector<typename Type::value_type>>;

    template <typename Type, typename ValueType>
    concept ArrayOrVectorOfType = ArrayOrVector<Type> && std::same_as<typename Type::value_type, ValueType>;
}