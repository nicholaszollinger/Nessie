#pragma once
// PerlinNoise.h
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
        Rng m_rng;
        Vector2f m_gradients[internal::kMaxPermutationCount];

    public:
        PerlinNoise2D();
        PerlinNoise2D(const uint64_t seed);
        ~PerlinNoise2D() = default;

        PerlinNoise2D(const PerlinNoise2D&) = delete;
        PerlinNoise2D(PerlinNoise2D&&) = delete;
        PerlinNoise2D& operator=(const PerlinNoise2D&) = delete;
        PerlinNoise2D& operator=(PerlinNoise2D&&) = delete;

    public:
        void Seed();
        void Seed(const uint64_t seed);
        uint64_t GetSeed() const;
        float GetNoise(float x, float y, uint32_t noiseInputRange, int octaves = 1, float persistence = 0.5f) const;

    private:
        float GetNoise(float noiseX, float noiseY) const;
    };
}