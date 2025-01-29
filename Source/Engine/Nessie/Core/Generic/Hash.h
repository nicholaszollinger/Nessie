// Hash.h
#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace nes
{
    template<typename HashType, typename HashableType>
    concept IsHashable = requires(HashType type, const HashableType& hashable)
    {
        { type(hashable) } -> std::same_as<uint64_t>;  
    };

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Algorithm:                              https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    //      InitialHash and PrimeMultiplier Values: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    //		
    ///		@brief : Generate an integral value based off a c-style string. Note: The return type is matched to the initial hash's type.
    ///		@param str : String we are hashing from.
    ///		@param initialHash : The initial 'offset' of the hash. This is the 'offset basis' value in the Fnv1a documentation. 
    ///		@param primeMultiplier : A prime number we multiply by. This is the 'FNV prime' value in the Fnv1a documentation.
    //-----------------------------------------------------------------------------------------------------------------------------
    template<typename Type> requires requires (Type type) { std::is_same_v<Type,uint32_t> || std::is_same_v<Type, uint64_t>; }
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
    //		NOTES:
    //      This uses the Fnv1aHash, as if the bytes are a string.
    //		
    ///		@brief : Generates a uint64_t number based on an array of bytes. 
    ///		@param bytes : Array.
    ///		@param size : Size of the Array.
    //-----------------------------------------------------------------------------------------------------------------------------
    constexpr uint64_t HashBytes(const std::byte* bytes, const int size)
    {
        constexpr uint64_t kInitialHash = 0xcbf29ce484222325ull;
        constexpr uint64_t kPrimeMultiplier = 0x100000001b3ull;

        if (!bytes)
            return kInitialHash;

        auto hash = kInitialHash;

        for (size_t i = 0; i < size; ++i)
        {
            hash ^= static_cast<uint64_t>(*bytes);
            hash *= kPrimeMultiplier;
            ++bytes;
        }

        return hash;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //      InitialHash and PrimeMultiplier Values: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    //
    ///		@brief : Generate an uint32_t number based on a c-string.
    //-----------------------------------------------------------------------------------------------------------------------------
    constexpr uint32_t HashString32(const char* str)
    {
        constexpr uint32_t kInitialHash = 0x811c9dc5;
        constexpr uint32_t kPrimeMultiplier = 0x01000193;

        return Fnv1aHashString(str, kInitialHash, kPrimeMultiplier);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //      InitialHash and PrimeMultiplier Values: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    //
    ///		@brief : Generate an uint64_t number based on a c-string.
    //-----------------------------------------------------------------------------------------------------------------------------
    constexpr uint64_t HashString64(const char* str)
    {
        constexpr uint64_t kInitialHash = 0xcbf29ce484222325ull;
        constexpr uint64_t kPrimeMultiplier = 0x100000001b3ull;

        return Fnv1aHashString(str, kInitialHash, kPrimeMultiplier);
    }
}
