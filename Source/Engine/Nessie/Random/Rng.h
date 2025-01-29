#pragma once
// Rng.h
#include <chrono>
#include <limits>

#include "Core/Generic/Concepts.h"
#include "Math/Vector2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Random Number Generator class. This can be used on its own thread.
    //----------------------------------------------------------------------------------------------------
    class RandomNumberGenerator
    {
        static constexpr uint64_t kRandMax = std::numeric_limits<uint64_t>::max();
	    static constexpr uint64_t kHalfRandMax = kRandMax >> 1;

        uint64_t m_state[4] { 0, 0, 0, 0 };
	    uint64_t m_lastSeedValue = 0; // Last Seed Value used to initialize the RNG.

    public:
        RandomNumberGenerator();
		RandomNumberGenerator(const uint64_t seed);
		~RandomNumberGenerator() = default;
	    RandomNumberGenerator(const RandomNumberGenerator&) = delete;
	    RandomNumberGenerator& operator=(const RandomNumberGenerator&) = delete;
	    RandomNumberGenerator(RandomNumberGenerator&& other) noexcept = default;
	    RandomNumberGenerator& operator=(RandomNumberGenerator&& other) noexcept = default;

	    void SeedFromTime();
        void SeedFromRand();
	    void SetSeed(uint64_t seed);
	    uint64_t GetLastSeed() const;

        uint64_t Rand();
	    bool RandBool();

        template <ScalarType Type>
	    Type RandRange(const Type min, const Type max);

	    template <UnsignedIntegralType Type = size_t>
	    Type RandIndex(const size_t size);

	    template <std::floating_point Type = float>
	    Type NormalizedRand();

	    template <std::floating_point Type = float>
	    Type SignedNormalizedRand();

	    template <std::floating_point Type = float>
	    TVector2<Type> RandUnitVector2D();

    private:
        void InitState(uint64_t seed);
    };

    using Rng = RandomNumberGenerator;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Return a random value between [min, max].
	///		@tparam Type : Type of the value to return, must be a Numeric type.
	///		@param min : Min value of the range.
	///		@param max : Max value of the range.
	//----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    Type Rng::RandRange(const Type min, const Type max)
    {
        // Floating point version:
	    if constexpr (std::floating_point<Type>)
	    {
	        const Type range = max - min;
	        return min + (range * NormalizedRand<Type>());
	    }

	    // Integral version:
	    else
	    {
	        return min + static_cast<Type>((Rand() % (max - min + 1)));
	    }
    }

    //----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Return a random index between [0, size - 1].
	///		@tparam Type : Type of the value to return, must be an integral type. Default is size_t.
	///		@param size : Size of the container.
	//----------------------------------------------------------------------------------------------------
    template <UnsignedIntegralType Type>
    Type Rng::RandIndex(const size_t size)
    {
        return static_cast<Type>(Rand() % size);
    }

    //----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Return a random floating point value between [0, 1].
	///		@tparam Type : Floating point type to return.
	//----------------------------------------------------------------------------------------------------
	template <std::floating_point Type>
	Type Rng::NormalizedRand()
	{
	    return static_cast<Type>(Rand()) / static_cast<Type>(kRandMax);
	}

    //----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Return a random floating point value between [-1, 1].
	///		@tparam Type : Floating point type to return.
	//----------------------------------------------------------------------------------------------------
	template <std::floating_point Type>
	Type Rng::SignedNormalizedRand()
	{
	    return (NormalizedRand<Type>() * static_cast<Type>(2)) - static_cast<Type>(1);
	}

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns a Random Unit Vector in 2D space.
    ///		@tparam Type : Floating point element type.
    //----------------------------------------------------------------------------------------------------
    template <std::floating_point Type>
    TVector2<Type> Rng::RandUnitVector2D()
    {
        const Type angle = NormalizedRand<Type>() * math::TwoPi<Type>();
        return { std::cos(angle), std::sin(angle) };
    }
}