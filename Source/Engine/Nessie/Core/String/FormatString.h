#pragma once
// FormatString.hpp
#include <chrono>
#include <ostream>
#include <string>

//---------------------------------------------------------------------------------------------------------------------------
// TODO: Most of this is fine, but I want to do the following when I can:
// - Printf style formatting. After living with this comma style for a while, Printf makes things more clear.
//    - Investigate the FString literal that was proposed for C++. (Probably too hard)
//    - Investigate Unreal's version of this.
//    - Example: FormatString("Hello %d, %.2f", 1, 3.14159); -> "Hello 1, 3.15"
//             - I want to be able to set how the float is formatted. (e.g. %.2f could be 3.14, %.3f could be 3.142 - notice the rounding!)
// - This should all be wrapped in a namespace!
// - Move the Current Time string functions to the Time module.
// - Move the Functor invocation functions to somewhere more Generic.
//---------------------------------------------------------------------------------------------------------------------------

namespace nes
{
    template <typename Type>
    concept OutStreamType = requires(std::ostream out, Type type)
    {
        out << type;
    };

    template <typename Type>
    concept StdToStringType = requires(Type type)
    {
        std::to_string(type);
    };

    template <typename Type>
    concept ToStringType = requires(const Type type)
    {
        { type.ToString() } -> std::same_as<std::string>;
    };

    template <typename Type>
    concept AddableToStringType = requires(std::string str, Type type)
    {
        str += type;
    };

    template <typename Type>
    concept AddableOrToStringType = AddableToStringType<Type> || ToStringType<Type> || StdToStringType<Type>;

    // TODO: Rename to ValidFunctorParams, <typename FunctorType, typename...Params>
    template <typename Type, typename...Args>
    concept InvokableType = requires(const Type type, Args&&...args)
    {
        type(args...);
    };

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //      TODO: I should move this somewhere more 'generic'.
    //		
    ///		@brief : Invokes each callable.
    ///		@tparam FunctorType : Callable type.
    ///     @tparam Args : Type of arguments passed to each callable.
    ///		@param funcs : Functors to be called.
    //-----------------------------------------------------------------------------------------------------------------------------
    void InvokeAll(const auto&&... funcs)
    {
        (funcs(), ...);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the size of a c-style string at compile time.
    //----------------------------------------------------------------------------------------------------
    constexpr size_t StrLength(const char* str)
    {
        const char* pCursor = str;
        while (*pCursor != '\0')
            ++pCursor;

        return pCursor - str;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Adds a value to a string.
    ///		@param str : String we are adding to.
    ///		@param arg : Value we are adding. This must be either an AddableToStringType or a ToStringType.
    //-----------------------------------------------------------------------------------------------------------------------------
    void AddToString(std::string& str, const AddableOrToStringType auto& arg)
    {
        using Type = std::remove_cvref_t<decltype(arg)>;

        if constexpr (StdToStringType<Type> && !std::is_same_v<char, Type>)
        {
            str += std::to_string(arg);
        }

        else if constexpr (ToStringType<Type>)
        {
            str += arg.ToString();
        }

        else
        {
            str += arg;
        }
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Concatenates each of the arg values into a single std::string.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string CombineIntoString(const AddableOrToStringType auto&...args)
    {
        std::string output{};

        // Source: https://en.cppreference.com/w/cpp/language/fold
        InvokeAll([&]() { AddToString(output, args); } ...);

        return output;
    }

    void FormatStringImpl(const char* pFormat, std::string& outStr);

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Combines arguments into a string using a format string.
    ///		@param pFormat : The format string that tells us when to place an argument.
    ///		@param outStr : The string we are outputting to.
    ///		@param first : The first argument, taken out of the parameter pack.
    ///		@param args : The rest of the arguments.
    //-----------------------------------------------------------------------------------------------------------------------------
    void FormatStringImpl(const char* pFormat, std::string& outStr, const AddableOrToStringType auto& first, const AddableOrToStringType auto&...args)
    {
        for (; *pFormat != '\0'; pFormat++)
        {
            if (*pFormat == '%')
            {
                ++pFormat;
                AddToString(outStr, first);
                FormatStringImpl(pFormat, outStr, args...);
                return;
            }

            outStr += *pFormat;
        }
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This based of the 'tprintf' function on cppreference: https://en.cppreference.com/w/cpp/language/parameter_pack
    //
    ///		@brief : Create a std::string that follows the format.
    ///         \n FORMAT RULES:
    ///         \n Anytime you want to insert an argument, put a '%' character into the formatted string.
    ///         \n Each argument following the format string will be added to the final string in order.
    ///         \n\n EXAMPLE:
    ///         \n FormatString("% world% %", "Hello", '!', 123); will print: "Hello World! 123".
    ///         \n (You can add a newline char to the format string, I can't here because of comment formatting.)\n
    ///		@tparam Args : Any type'd arguments that you want to add into the std::string.
    ///		@param pFormat : String that defines the format of the final output.
    ///		@param args : Values to add into the string in sequential order.
    ///		@returns : Formatted string.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string FormatString(const char* pFormat, const AddableOrToStringType auto&...args)
    {
        std::string output;
        FormatStringImpl(pFormat, output, args...);
        return output;
    }
}