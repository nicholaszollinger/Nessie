// UUID.h
#pragma once
#include "Rng.h"

namespace nes
{
    class UUID
    {
    public:
        using ValueType = uint64;
        
        /// Constructors
        constexpr           UUID() = default;
        constexpr           UUID(const uint64 value) : m_value(value) {}

        /// Operators
        constexpr bool      operator==(const UUID& other) const { return m_value == other.m_value; }
        constexpr bool      operator!=(const UUID& other) const { return m_value != other.m_value; }

        constexpr ValueType GetValue() const                    { return m_value; }
        constexpr bool      IsValid() const                     { return m_value != kInvalidValue; }

    private:
        static constexpr ValueType kInvalidValue = 0;
        ValueType           m_value = kInvalidValue;
    };

    class UUIDGenerator
    {
    public:
        UUIDGenerator() : m_rng() {}
        UUIDGenerator(const uint64 seed) : m_rng(seed) {}
        UUIDGenerator(const UUIDGenerator&) = delete;
        UUIDGenerator& operator=(const UUIDGenerator&) = delete;
        UUIDGenerator(UUIDGenerator&& other) noexcept = default;
        UUIDGenerator& operator=(UUIDGenerator&& other) noexcept = default;

        UUID GenerateUUID() { return m_rng.Rand(); }

    private:
        Rng m_rng;
    };

    struct UUIDHasher
    {
        uint64 operator()(const UUID id) const
        {
            return id.GetValue();
        }
    };
}
