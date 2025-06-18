// StringID.h
#pragma once
#include <unordered_map>
#include "FormatString.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    //	NOTES:
    //  This is an interned string implementation. I did the simple thing and just had the string database be a static member
    //  on the class. That means that the database could get HUGE, and I would prefer to have a 'scope' or something (like
    //  GameObjectName StringIds) but this was the simple thing.
    //
    ///	@brief : A StringId is a pointer to a std::string that lives in a static database. Comparing StringIds is trivial because
    ///     we are just comparing the pointers.
    //-----------------------------------------------------------------------------------------------------------------------------
    class StringID
    {
        // Easiest
        using StringContainer = std::unordered_map<uint64_t, std::string>;

    public:
        /// Constructors
        StringID();
        StringID(const char* str);
        StringID(const std::string& str);
        StringID(const StringID& right) = default;
        StringID(StringID&& right) noexcept;
        ~StringID() = default;

        /// Assignment Operators
        StringID& operator=(const StringID& right) = default;
        StringID& operator=(StringID&& right) noexcept;
        
        /// Operators
        const std::string&      operator*() const;
        constexpr bool          operator==(const StringID& right) const { return m_pStrRef == right.m_pStrRef; }
        constexpr bool          operator!=(const StringID& right) const { return !(*this == right); }

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Invalid StringID. 
        //----------------------------------------------------------------------------------------------------
        static StringID         GetInvalidID();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get a copy of the internal string. If you just want a reference, use GetStringRef().
        /// @note : NOT THREAD SAFE.
        //----------------------------------------------------------------------------------------------------
        std::string             StringCopy() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a const reference to the internal string. If you want a copy, use GetStringCopy().
        /// @note : NOT THREAD SAFE.
        //----------------------------------------------------------------------------------------------------
        const std::string&      StringRef() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the underlying std::string as a const ptr. 
        //----------------------------------------------------------------------------------------------------
        const std::string*      ConstPtr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the underlying std::string as a c-string. 
        //----------------------------------------------------------------------------------------------------
        const char*             CStr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether the underlying string the Invalid String. Same as *this == GetInvalidID(). 
        //----------------------------------------------------------------------------------------------------
        bool                    IsValid() const { return m_pStrRef != GetInvalidStringAddress(); }

    private:
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Either returns the address to an existing string in the static container, creates a
        ///     new entry in the container for this string, or returns the invalid string address in the
        ///     event that nullptr was passed in.
        //----------------------------------------------------------------------------------------------------
        static std::string*     MakeStringPtr(const char* str);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Either returns the address to an existing string in the static container, creates a
        ///     new entry in the container for this string, or returns the invalid string address in the
        ///     event that an empty string was passed in.
        //----------------------------------------------------------------------------------------------------
        static std::string*     MakeStringPtr(const std::string& str);
        static StringContainer& GetContainer();
        static std::string*     GetInvalidStringAddress();

    private:
        std::string* m_pStrRef = nullptr;
    };

    struct StringIDHasher
    {
        uint64_t operator()(const StringID id) const;
    };

    std::string& operator+=(std::string& str, const StringID& stringId);

}