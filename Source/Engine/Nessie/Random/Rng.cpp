// Rng.cpp

#include "Rng.h"
#include "Core/Time/Time.h"

// [TODO]: Move this to Bit.h
static constexpr uint64_t RotateLeft(const uint64_t value, int k)
{
    return (value << k) | (value >> (64 - k));
}

static constexpr uint64_t SplitMix64(uint64_t& seed)
{
    // https://en.wikipedia.org/wiki/Xorshift
    uint64_t result = seed += 0x9E3779B97F4A7C15;
    result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

namespace nes
{
    Rng::RandomNumberGenerator()
    {
        SeedFromTime();
    }

    Rng::RandomNumberGenerator(const uint64_t seed)
    {
        SetSeed(seed);
    }
	
    void Rng::SeedFromTime()
    {
        const uint64_t seed = Time::Now();
        InitState(seed);
    }
	
    void Rng::SeedFromRand()
    {
        const auto seed = Rand();
        InitState(seed);
    }
	
    void Rng::SetSeed(uint64_t seed)
    {
        InitState(seed);
    }
	
    uint64_t RandomNumberGenerator::GetLastSeed() const
    {
        return m_lastSeedValue;
    }
	
    uint64_t Rng::Rand()
    {
        // Xoshiro256** algorithm: https://en.wikipedia.org/wiki/Xorshift
	    const uint64_t result = RotateLeft(m_state[1] * 5, 7) * 9;

	    const uint64_t t = m_state[1] << 17;

	    m_state[2] ^= m_state[0];
	    m_state[3] ^= m_state[1];
	    m_state[1] ^= m_state[2];
	    m_state[0] ^= m_state[3];

	    m_state[2] ^= t;
	    m_state[3] = RotateLeft(m_state[3], 45);

	    return result;
    }
	
    bool Rng::RandBool()
    {
        return Rand() < kHalfRandMax;
    }

    //----------------------------------------------------------------------------------------------------
	///		@brief : Initialize the RNG state with a specific seed. This makes it is "good" seed.
	//----------------------------------------------------------------------------------------------------
    void Rng::InitState(uint64_t seed)
    {
        m_lastSeedValue = seed;

	    m_state[0] = SplitMix64(seed);
	    m_state[1] = SplitMix64(seed);
	    m_state[2] = SplitMix64(seed);
	    m_state[3] = SplitMix64(seed);
    }

	Vec2 RandomNumberGenerator::RandUnitVector2()
	{
    	const float angle = NormalizedRand<float>() * math::TwoPi<float>();
    	return { std::cos(angle), std::sin(angle) };
	}

}