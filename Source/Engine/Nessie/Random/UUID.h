#pragma once
// UUID.h

#include "Rng.h"

namespace nes
{
    class UUID
    {
        static constexpr size_t kInvalidValue = 0;
        uint64_t m_value = kInvalidValue;

    public:
        constexpr UUID() = default;
        constexpr UUID(const uint64_t value) : m_value(value) {}

        constexpr bool operator==(const UUID& other) const { return m_value == other.m_value; }
        constexpr bool operator!=(const UUID& other) const { return m_value != other.m_value; }

        constexpr uint64_t GetValue() const { return m_value; }
        constexpr bool IsValid() const { return m_value != kInvalidValue; }
    };

    class UUIDGenerator
    {
        Rng m_rng;

    public:
        UUIDGenerator() : m_rng() {}
        UUIDGenerator(const uint64_t seed) : m_rng(seed) {}
        UUIDGenerator(const UUIDGenerator&) = delete;
        UUIDGenerator& operator=(const UUIDGenerator&) = delete;
        UUIDGenerator(UUIDGenerator&& other) noexcept = default;
        UUIDGenerator& operator=(UUIDGenerator&& other) noexcept = default;

        UUID GenerateUUID() { return m_rng.Rand(); }
    };

    struct UUIDHasher
    {
        uint64_t operator()(const UUID id) const
        {
            return id.GetValue();
        }
    };
}
