// Hash.h
#pragma once
#include <concepts>
#include <type_traits>
#include "Core/Config.h"

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
        if (!str)
            return initialHash;

        auto hash = initialHash;

        while (*str != '\0')
        {
            hash ^= static_cast<Type>(*str);
            hash *= primeMultiplier;
            ++str;
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

        if (!bytes)
            return kInitialHash;

        auto hash = kInitialHash;

        for (size_t i = 0; i < size; ++i)
        {
            hash ^= static_cast<uint64>(*bytes);
            hash *= kPrimeMultiplier;
            ++bytes;
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
}
