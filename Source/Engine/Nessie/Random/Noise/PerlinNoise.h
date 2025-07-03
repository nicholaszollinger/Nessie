// PerlinNoise.h
#pragma once
#include "../Rng.h"

namespace nes
{
    namespace internal
    {
        constexpr uint32_t kMaxPermutationCount = 256;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      - I got help from the following sources:
    //         https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2/perlin-noise.html
    //         https://www.redblobgames.com/maps/terrain-from-noise/
    //		
    ///		@brief : Class for generating 2D Perlin Noise.
    //----------------------------------------------------------------------------------------------------
    class PerlinNoise2D
    {
    public:
        PerlinNoise2D();
        PerlinNoise2D(const uint64 seed);
        ~PerlinNoise2D() = default;

        PerlinNoise2D(const PerlinNoise2D&) = delete;
        PerlinNoise2D(PerlinNoise2D&&) = delete;
        PerlinNoise2D& operator=(const PerlinNoise2D&) = delete;
        PerlinNoise2D& operator=(PerlinNoise2D&&) = delete;

    public:
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Seed the Noise Generator. This is an expensive operation, so try not to do this often.
        //----------------------------------------------------------------------------------------------------
        void    Seed();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Seed the Noise Generator. This is an expensive operation, so try not to do this often.
        //----------------------------------------------------------------------------------------------------
        void    Seed(const uint64 seed);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the Seed value used to initialize PerlinNoise.
        //----------------------------------------------------------------------------------------------------
        uint64  GetSeed() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get a Noise value at a given 2D position.
        ///	@param x : X Position.
        ///	@param y : Y Position.
        ///	@param noiseInputRange : Determines the 'size' of the noise grid.
        ///	@param octaves : Lower values will be smoother, higher values will be more detailed.
        ///	@param persistence : Determines how much each octave contributes to the overall noise.
        ///     The default value is 0.5f, meaning that each layer contributes evenly to the next.
        ///	@returns : Noise value in the range [0, 1].
        //----------------------------------------------------------------------------------------------------
        float   CalculateNoise(float x, float y, uint32 noiseInputRange, int octaves = 1, float persistence = 0.5f) const;

    private:
        float   CalculateNoise(float noiseX, float noiseY) const;

    private:
        Rng     m_rng;
        Vec2    m_gradients[internal::kMaxPermutationCount];
    };
}