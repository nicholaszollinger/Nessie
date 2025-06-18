// UUID.h
#pragma once
#include "Rng.h"

namespace nes
{
    class UUID
    {
    public:
        /// Constructors
        constexpr           UUID() = default;
        constexpr           UUID(const uint64 value) : m_value(value) {}

        /// Operators
        constexpr bool      operator==(const UUID& other) const { return m_value == other.m_value; }
        constexpr bool      operator!=(const UUID& other) const { return m_value != other.m_value; }

        constexpr uint64    GetValue() const                    { return m_value; }
        constexpr bool      IsValid() const                     { return m_value != kInvalidValue; }

    private:
        static constexpr size_t kInvalidValue = 0;
        uint64_t m_value = kInvalidValue;
    };

    class UUIDGenerator
    {
    public:
        UUIDGenerator() : m_rng() {}
        UUIDGenerator(const uint64_t seed) : m_rng(seed) {}
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
        uint64_t operator()(const UUID id) const
        {
            return id.GetValue();
        }
    };
}
