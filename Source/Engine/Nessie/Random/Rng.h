#pragma once
// Rng.h
#include <chrono>
#include <limits>
#include "Core/Generic/Concepts.h"
#include "Math/Vec2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Random Number Generator class. This can be used on its own thread.
    //----------------------------------------------------------------------------------------------------
    class RandomNumberGenerator
    {
    public:
        static constexpr uint64_t kRandMax = std::numeric_limits<uint64_t>::max();
	    static constexpr uint64_t kHalfRandMax = kRandMax >> 1;

    public:
        RandomNumberGenerator();
		RandomNumberGenerator(const uint64_t seed);
		~RandomNumberGenerator() = default;
	    RandomNumberGenerator(const RandomNumberGenerator&) = delete;
	    RandomNumberGenerator& operator=(const RandomNumberGenerator&) = delete;
	    RandomNumberGenerator(RandomNumberGenerator&& other) noexcept = default;
	    RandomNumberGenerator& operator=(RandomNumberGenerator&& other) noexcept = default;

    	//----------------------------------------------------------------------------------------------------
    	///		@brief : Seed the RNG with a random value based on the current time.
    	//----------------------------------------------------------------------------------------------------
	    void 	SeedFromTime();

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Seed the RNG with the result from Rand().
    	//----------------------------------------------------------------------------------------------------
        void 	SeedFromRand();

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Seed the RNG with a given value.
    	//----------------------------------------------------------------------------------------------------
	    void 	SetSeed(uint64 seed);

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Returns the last seed value used to initialize the RNG. Calls to Seed() will
    	///		change this value.
    	//----------------------------------------------------------------------------------------------------
	    uint64	GetLastSeed() const;

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Returns a random UINT64 value.
    	//----------------------------------------------------------------------------------------------------
        uint64	Rand();

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Returns a random boolean value.
    	//----------------------------------------------------------------------------------------------------
	    bool	RandBool();

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Return a random value between [min, max].
    	///	@tparam Type : Type of the value to return, must be a Numeric type.
    	//----------------------------------------------------------------------------------------------------
        template <ScalarType Type>
	    Type	RandRange(const Type min, const Type max);

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Return a random index between [0, size - 1].
    	///	@tparam Type : Type of the value to return, must be an integral type. Default is size_t.
    	///	@param size : Size of the container.
    	//----------------------------------------------------------------------------------------------------
	    template <UnsignedIntegralType Type = size_t>
	    Type	RandIndex(const size_t size)				{ return static_cast<Type>(Rand() % size); }

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Return a random floating point value between [0, 1].
    	//----------------------------------------------------------------------------------------------------
	    template <std::floating_point Type = float>
	    Type	NormalizedRand()							{ return static_cast<Type>(Rand()) / static_cast<Type>(kRandMax); }

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Return a random floating point value between [-1, 1].
    	//----------------------------------------------------------------------------------------------------
	    template <std::floating_point Type = float>
	    Type	SignedNormalizedRand()						{ return (NormalizedRand<Type>() * static_cast<Type>(2)) - static_cast<Type>(1); }

    	//----------------------------------------------------------------------------------------------------
    	///	@brief : Returns a random unit vector in 2D space.
    	//----------------------------------------------------------------------------------------------------
	    Vec2	RandUnitVector2();

    private:
        void	InitState(uint64_t seed);

    private:
    	uint64 	m_state[4] { 0, 0, 0, 0 };
    	uint64 	m_lastSeedValue = 0; // Last Seed Value used to initialize the RNG.
    };

    using Rng = RandomNumberGenerator;
}

namespace nes
{
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
}