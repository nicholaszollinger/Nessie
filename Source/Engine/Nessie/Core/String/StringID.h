#pragma once
// StringID.hpp
#include <unordered_map>
#include "FormatString.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is an interned string implementation. I did the simple thing and just had the string database be a static member
    //      on the class. That means that the database could get HUGE, and I would prefer to have a 'scope' or something (like
    //      GameObjectName StringIds) but this was the simple thing.
    //
    ///		@brief : A StringId is a pointer to a std::string that lives in a static database. Comparing StringIds is trivial because
    ///         we are just comparing the pointers.
    //-----------------------------------------------------------------------------------------------------------------------------
    class StringID
    {
        // Easiest
        using StringContainer = std::unordered_map<uint64_t, std::string>;
        std::string* m_pStrRef = nullptr;

    public:
        StringID();
        StringID(const char* str);
        StringID(const std::string& str);

        StringID(const StringID& right) = default;
        StringID(StringID&& right) noexcept;
        StringID& operator=(const StringID& right) = default;
        StringID& operator=(StringID&& right) noexcept;
        ~StringID() = default;

    public:
        const std::string& operator*() const;
        constexpr bool operator==(const StringID& right) const { return m_pStrRef == right.m_pStrRef; }
        constexpr bool operator!=(const StringID& right) const { return !(*this == right); }

    public:
        [[nodiscard]] static StringID GetInvalid();
        [[nodiscard]] std::string GetStringCopy() const;
        [[nodiscard]] const std::string& GetStringRef() const;
        [[nodiscard]] const std::string* GetConstPtr() const;
        [[nodiscard]] const char* GetCStr() const;
        [[nodiscard]] bool IsValid() const { return m_pStrRef != GetInvalidStringAddress(); }

    private:
        static std::string* GetStringPtr(const char* str);
        static std::string* GetStringPtr(const std::string& str);

        static StringContainer& GetContainer();
        static std::string* GetInvalidStringAddress();
    };

    struct StringIDHasher
    {
        uint64_t operator()(const StringID id) const;
    };

    std::string& operator+=(std::string& str, const StringID& stringId);

}