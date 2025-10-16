// FormatString.h
#pragma once
#include <chrono>
#include <string>
#include "fmt/format.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Define a formatter to create a string version of the type. Following the format string parameter,
///     you need to supply the type's member variables that you are using for arguments in the string.
///     For each argument, use 'val.m_memberVar'.
///
///     Example usage:
///     <code>
///     NES_DEFINE_STRING_FORMATTER(nes::Vector4, "(x={:.3f}, y={:.3f}, z={:.3f}, w={:.3f})", val.x, val.y, val.z, val.w);
///     </code>
///
///	@param type : Type (including namespace) you are creating a formatter for. Ex: nes::Vec4  
///	@param formatStr : Format string to use for the type.
///     Example for nes::Vec4:
///     "(x={:.3f}, y={:.3f}, z={:.3f}, w={:.3f})"
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_STRING_FORMATTER(type, formatStr, ...)                       \
template <>                                                                     \
struct fmt::formatter<type> : fmt::formatter<std::string>                       \
{                                                                               \
    auto format(type val, format_context& ctx) const -> decltype(ctx.out())     \
    {                                                                           \
        return fmt::format_to(ctx.out(), formatStr, __VA_ARGS__);               \
    }                                                                           \
}

//----------------------------------------------------------------------------------------------------
/// @brief : Begin a format block for an enum. Combined with NES_FORMAT_ENUM_CASE() and NES_END_ENUM_FORMATTER(),
///     this defines a class that will format the enum's values so that it prints out the value name
///     in a format string. ETest::True -> "True".
///
/// Example:
///
/// <code>
/// enum class ETest
/// {
///     False,
///     True,
///     Maybe
/// };
///
/// NES_BEGIN_ENUM_FORMATTER(ETest)
///     NES_FORMAT_ENUM_CASE(False)
///     NES_FORMAT_ENUM_CASE(True)
/// NES_END_ENUM_FORMATTER()
/// </code>
///
/// - nes::ToString(ETest::True) -> "True"
/// - nes::ToString(ETest::False) -> "False"
/// - nes::ToString(ETest::Maybe) -> "Unknown" This is the default case when not specified.
///
///	@param enumType : Enum Type, ex: 
//----------------------------------------------------------------------------------------------------
#define NES_BEGIN_ENUM_FORMATTER(enumType)                                                  \
    template <>                                                                             \
    struct fmt::formatter<enumType> : fmt::formatter<std::string>                           \
    {                                                                                       \
        using Type = enumType;                                                              \
        auto format(enumType value, format_context& ctx) const -> format_context::iterator  \
        {                                                                                   \
            std::string_view name = "Unknown";                                              \
            switch (value)                                                                  \
            { 

//----------------------------------------------------------------------------------------------------
/// @brief : Bookend macro for NES_BEGIN_ENUM_FORMATTER().
///     See NES_BEGIN_ENUM_FORMATTER for example usage.
//----------------------------------------------------------------------------------------------------
#define NES_END_ENUM_FORMATTER()                                                            \
                default: break;                                                             \
            }                                                                               \
                                                                                            \
            return formatter<string_view>::format(name, ctx);                               \
        }                                                                                   \
    };

//----------------------------------------------------------------------------------------------------
/// @brief : Format an individual enum case value.
///     See NES_BEGIN_ENUM_FORMATTER for example usage.
//----------------------------------------------------------------------------------------------------
#define NES_FORMAT_ENUM_CASE(enumValue) case Type::enumValue: name = #enumValue; break;

namespace nes
{
    /// Is valid if NES_DEFINE_STRING_FORMATTER() was set for the type. 
    template <typename Type>
    concept FormattableType = requires(Type type)
    {
        { fmt::formatter<Type>() } -> std::convertible_to<fmt::formatter<std::string>>;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create a string representation of the type.
    //----------------------------------------------------------------------------------------------------
    template <FormattableType Type>
    std::string ToString(const Type& type)
    {
        fmt::memory_buffer buffer;
        fmt::format_to(std::back_inserter(buffer), "{}", type);
        return fmt::to_string(buffer);
    }
}

namespace nes
{
    inline std::string StripNamespaceFromTypename(const std::string_view fullName)
    {
        const size_t lastColon = fullName.find_last_of(':');
        if (lastColon != std::string_view::npos)
        {
            return std::string(fullName.substr(lastColon + 1));
        }
        return std::string(fullName);
    }
    
    // template <typename Type>
    // concept OutStreamType = requires(std::ostream out, Type type)
    // {
    //     out << type;
    // };
    //
    // template <typename Type>
    // concept StdToStringType = requires(Type type)
    // {
    //     std::to_string(type);
    // };
    //
    // template <typename Type>
    // concept ToStringType = requires(const Type type)
    // {
    //     { type.ToString() } -> std::same_as<std::string>;
    // };
    //
    // template <typename Type>
    // concept AddableToStringType = requires(std::string str, Type type)
    // {
    //     str += type;
    // };
    //
    // template <typename Type>
    // concept AddableOrToStringType = AddableToStringType<Type> || ToStringType<Type> || StdToStringType<Type>;
    //
    // // TODO: Rename to ValidFunctorParams, <typename FunctorType, typename...Params>
    // template <typename Type, typename...Args>
    // concept InvokableType = requires(const Type type, Args&&...args)
    // {
    //     type(args...);
    // };
    //
    // //-----------------------------------------------------------------------------------------------------------------------------
    // //	NOTES:
    // //  TODO: I should move this somewhere more 'generic'.
    // //		
    // /// @brief : Invokes each callable.
    // ///	@tparam FunctorType : Callable type.
    // /// @tparam Args : Type of arguments passed to each callable.
    // ///	@param funcs : Functors to be called.
    // //-----------------------------------------------------------------------------------------------------------------------------
    // void InvokeAll(const auto&&... funcs)
    // {
    //     (funcs(), ...);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///	@brief : Calculate the size of a c-style string at compile time.
    // //----------------------------------------------------------------------------------------------------
    // constexpr size_t StrLength(const char* str)
    // {
    //     const char* pCursor = str;
    //     while (*pCursor != '\0')
    //         ++pCursor;
    //
    //     return pCursor - str;
    // }
    //
    // //-----------------------------------------------------------------------------------------------------------------------------
    // ///	@brief : Adds a value to a string.
    // ///	@param str : String we are adding to.
    // ///	@param arg : Value we are adding. This must be either an AddableToStringType or a ToStringType.
    // //-----------------------------------------------------------------------------------------------------------------------------
    // void AddToString(std::string& str, const AddableOrToStringType auto& arg)
    // {
    //     using Type = std::remove_cvref_t<decltype(arg)>;
    //
    //     if constexpr (StdToStringType<Type> && !std::is_same_v<char, Type>)
    //     {
    //         str += std::to_string(arg);
    //     }
    //
    //     else if constexpr (ToStringType<Type>)
    //     {
    //         str += arg.ToString();
    //     }
    //
    //     else
    //     {
    //         str += arg;
    //     }
    // }
    //
    // //-----------------------------------------------------------------------------------------------------------------------------
    // ///	@brief : Concatenates each of the arg values into a single std::string.
    // //-----------------------------------------------------------------------------------------------------------------------------
    // std::string CombineIntoString(const AddableOrToStringType auto&...args)
    // {
    //     std::string output{};
    //
    //     // Source: https://en.cppreference.com/w/cpp/language/fold
    //     InvokeAll([&]() { AddToString(output, args); } ...);
    //
    //     return output;
    // }
    //
    // void FormatStringImpl(const char* pFormat, std::string& outStr);
    //
    // //-----------------------------------------------------------------------------------------------------------------------------
    // ///	@brief : Combines arguments into a string using a format string.
    // ///	@param pFormat : The format string that tells us when to place an argument.
    // ///	@param outStr : The string we are outputting to.
    // ///	@param first : The first argument, taken out of the parameter pack.
    // ///	@param args : The rest of the arguments.
    // //-----------------------------------------------------------------------------------------------------------------------------
    // void FormatStringImpl(const char* pFormat, std::string& outStr, const AddableOrToStringType auto& first, const AddableOrToStringType auto&...args)
    // {
    //     for (; *pFormat != '\0'; pFormat++)
    //     {
    //         if (*pFormat == '%')
    //         {
    //             ++pFormat;
    //             AddToString(outStr, first);
    //             FormatStringImpl(pFormat, outStr, args...);
    //             return;
    //         }
    //
    //         outStr += *pFormat;
    //     }
    // }
    //
    // //-----------------------------------------------------------------------------------------------------------------------------
    // //	NOTES:
    // //  This based of the 'printf' function on cppreference: https://en.cppreference.com/w/cpp/language/parameter_pack
    // //
    // ///	@brief : Create a string that follows the format.
    // ///     \n FORMAT RULES:
    // ///     \n - Anytime you want to insert an argument, put a '%' character into the formatted string.
    // ///     \n - Each argument following the format string will be added to the final string in order.
    // ///     \n\n EXAMPLE:
    // ///     \n FormatString("% world% %", "Hello", '!', 123); will print: "Hello World! 123".
    // ///     \n (You can add a newline char to the format string, I can't here because of comment formatting.)\n
    // ///	@tparam Args : Any typed arguments that you want to add into the std::string.
    // ///	@param pFormat : String that defines the format of the final output.
    // ///	@param args : Values to add into the string in sequential order.
    // ///	@returns : Formatted string.
    // //-----------------------------------------------------------------------------------------------------------------------------
    // std::string FormatString(const char* pFormat, const AddableOrToStringType auto&...args)
    // {
    //     std::string output;
    //     FormatStringImpl(pFormat, output, args...);
    //     return output;
    // }
}