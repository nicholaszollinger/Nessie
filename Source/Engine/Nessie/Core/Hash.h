// Hash.h
#pragma once
#include <concepts>
#include <type_traits>
#include "Nessie/Core/Config.h"

namespace nes
{
    template <typename HashType, typename HashableType>
    concept IsHashable = requires(HashType type, const HashableType& hashable)
    {
        { type(hashable) } -> std::same_as<uint64_t>;  
    };

    //-----------------------------------------------------------------------------------------------------------------------------
    //	NOTES:
    //  Algorithm:                              https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    //  InitialHash and PrimeMultiplier Values: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    //		
    ///	@brief : Generate an integral value based off a c-style string. Note: The return type is matched to the initial hash's type.
    ///	@param str : String we are hashing from.
    ///	@param initialHash : The initial 'offset' of the hash. This is the 'offset basis' value in the Fnv1a documentation. 
    ///	@param primeMultiplier : A prime number we multiply by. This is the 'FNV prime' value in the Fnv1a documentation.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <typename Type> requires requires (Type type) { std::is_same_v<Type, uint32> || std::is_same_v<Type, uint64>; }
    constexpr Type Fnv1aHashString(const char* str, const Type initialHash, const Type primeMultiplier)
    {
        auto hash = initialHash;
        for (const char* c = str; *c != 0; ++c)
        {
            hash ^= static_cast<Type>(*c);
            hash *= primeMultiplier;
        }

        return hash;
    }
    
    //-----------------------------------------------------------------------------------------------------------------------------
    // This uses the Fnv1aHash, as if the bytes are a string.
    //		
    ///	@brief : Generates a uint64 number based on an array of bytes. 
    ///	@param bytes : Array.
    ///	@param size : Size of the Array.
    //-----------------------------------------------------------------------------------------------------------------------------
    constexpr uint64 HashBytes(const std::byte* bytes, const size_t size)
    {
        constexpr uint64 kInitialHash = 0xcbf29ce484222325ull;
        constexpr uint64 kPrimeMultiplier = 0x100000001b3ull;
        
        auto hash = kInitialHash;
        for (const std::byte* pData = bytes; pData < bytes + size; ++pData)
        {
            hash ^= static_cast<uint64>(*pData);
            hash *= kPrimeMultiplier;
        }

        return hash;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    // InitialHash and PrimeMultiplier Values: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    //
    /// @brief : Generate an uint32 number based on a c-string.
    //-----------------------------------------------------------------------------------------------------------------------------
    constexpr uint32 HashString32(const char* str)
    {
        constexpr uint32 kInitialHash = 0x811c9dc5;
        constexpr uint32 kPrimeMultiplier = 0x01000193;

        return Fnv1aHashString(str, kInitialHash, kPrimeMultiplier);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //  InitialHash and PrimeMultiplier Values: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    //
    /// @brief : Generate an uint64 number based on a c-string.
    //-----------------------------------------------------------------------------------------------------------------------------
    constexpr uint64 HashString64(const char* str)
    {
        constexpr uint64 kInitialHash = 0xcbf29ce484222325ull;
        constexpr uint64 kPrimeMultiplier = 0x100000001b3ull;

        return Fnv1aHashString(str, kInitialHash, kPrimeMultiplier);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : A 64-bit hash function by Thomas Wang, Jan 1997
    /// @see: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
    //----------------------------------------------------------------------------------------------------
    constexpr uint64 Hash64(const uint64 value)
    {
        uint64 hash = value;
        hash = (~hash) + (hash << 21); // hash = (hash << 21) - hash - 1;
        hash = hash ^ (hash >> 24);
        hash = (hash + (hash << 3)) + (hash << 8); // hash * 265
        hash = hash ^ (hash >> 14);
        hash = (hash + (hash << 2)) + (hash << 4); // hash * 21
        hash = hash ^ (hash >> 28);
        hash = hash + (hash << 31);
        return hash;
    }
}
