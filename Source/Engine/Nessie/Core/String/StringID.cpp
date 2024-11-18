// StringID.cpp

#include "StringID.h"
#include <mutex>
#include "Core/Generic/Hash.h"

static std::mutex& GetStringMutex()
{
    static std::mutex s_stringMutex;
    return s_stringMutex;
}

namespace nes
{
    StringID::StringID()
        : m_pStrRef(GetInvalidStringAddress())
    {
        //
    }

    StringID::StringID(const char* str)
        : m_pStrRef(GetStringPtr(str))
    {
        //
    }

    StringID::StringID(const std::string& str)
        : m_pStrRef(GetStringPtr(str))
    {
        //
    }

    StringID::StringID(StringID&& right) noexcept
        : m_pStrRef(right.m_pStrRef)
    {
        right.m_pStrRef = GetInvalidStringAddress();
    }

    StringID& StringID::operator=(StringID&& right) noexcept
    {
        if (right != *this)
        {
            m_pStrRef = right.m_pStrRef;
            right.m_pStrRef = GetInvalidStringAddress();
        }

        return *this;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Get a copy of the internal string. If you just want a reference, use GetStringRef(). \n NOT THREAD SAFE.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string StringID::GetStringCopy() const
    {
        return *m_pStrRef;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get a const reference to the internal string. If you want a copy, use GetStringCopy(). \n NOT THREAD SAFE.
    //-----------------------------------------------------------------------------------------------------------------------------
    const std::string& StringID::GetStringRef() const
    {
        return *m_pStrRef;
    }

    const std::string& StringID::operator*() const
    {
        return *m_pStrRef;
    }

    const std::string* StringID::GetConstPtr() const
    {
        return m_pStrRef;
    }

    const char* StringID::GetCStr() const
    {
        if (!IsValid())
            return nullptr;

        return m_pStrRef->c_str();
    }

    std::string* StringID::GetStringPtr(const char* str)
    {
        // If we are being set to nullptr, then return the address of the invalid string.
        if (!str)
            return GetInvalidStringAddress();

        const uint32_t hash = HashString32(str);

        std::lock_guard lock(GetStringMutex());

        auto& strings = GetContainer();

        // If we don't have the string already, add it to our container.
        if (!strings.contains(hash))
        {
            strings[hash] = str;
        }

        return &strings[hash];
    }

    std::string* StringID::GetStringPtr(const std::string& str)
    {
        if (str.empty())
            return GetInvalidStringAddress();

        return GetStringPtr(str.c_str());
    }

    StringID::StringContainer& StringID::GetContainer()
    {
        static StringContainer container;
        return container;
    }

    StringID StringID::GetInvalid()
    {
        static StringID invalid = "Invalid StringId";
        return invalid;
    }

    std::string* StringID::GetInvalidStringAddress()
    {
        return GetInvalid().m_pStrRef;
    }

    uint64_t StringIDHasher::operator()(const StringID id) const
    {
        static constexpr std::hash<const std::string*> kHash;
        return kHash(id.GetConstPtr());
    }

}
