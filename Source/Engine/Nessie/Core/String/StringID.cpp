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
        : m_pStrRef(MakeStringPtr(str))
    {
        //
    }

    StringID::StringID(const std::string& str)
        : m_pStrRef(MakeStringPtr(str))
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
    
    std::string StringID::StringCopy() const
    {
        return *m_pStrRef;
    }
    
    const std::string& StringID::StringRef() const
    {
        return *m_pStrRef;
    }

    const std::string& StringID::operator*() const
    {
        return *m_pStrRef;
    }

    const std::string* StringID::ConstPtr() const
    {
        return m_pStrRef;
    }

    const char* StringID::CStr() const
    {
        if (!IsValid())
            return nullptr;

        return m_pStrRef->c_str();
    }
    
    std::string* StringID::MakeStringPtr(const char* str)
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
    
    std::string* StringID::MakeStringPtr(const std::string& str)
    {
        if (str.empty())
            return GetInvalidStringAddress();

        return MakeStringPtr(str.c_str());
    }

    StringID::StringContainer& StringID::GetContainer()
    {
        static StringContainer container;
        return container;
    }

    StringID StringID::GetInvalidID()
    {
        static StringID invalid = "Invalid StringId";
        return invalid;
    }

    std::string* StringID::GetInvalidStringAddress()
    {
        return GetInvalidID().m_pStrRef;
    }

    uint64_t StringIDHasher::operator()(const StringID id) const
    {
        static constexpr std::hash<const std::string*> kHash;
        return kHash(id.ConstPtr());
    }
}
