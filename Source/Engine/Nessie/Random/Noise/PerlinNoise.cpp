// PerlinNoise.cpp

#include "PerlinNoise.h"

namespace nes
{
    namespace internal
    {
        static constexpr uint32_t kMaxTableSizeMask = kMaxPermutationCount - 1;

        // random permutation of 256 numbers, repeated 2x
	    static constexpr int kPermutation[kMaxPermutationCount * 3] = 
        {
		    63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
		    78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
		    57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
		    124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
		    104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
		    155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235, 

		    63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
		    78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
		    57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
		    124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
		    104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
		    155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235,

            63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
		    78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
		    57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
		    124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
		    104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
		    155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235
	    };

        static constexpr int GetPermutationValue2D(const int x, const int y)
        {
            const int pX = x & kMaxTableSizeMask;
            const int pY = y & kMaxTableSizeMask;
            return kPermutation[kPermutation[pX] + pY];
        }

        static constexpr int GetPermutationValue3D(const int x, const int y, const int z)
        {
            const int pX = x & kMaxTableSizeMask;
            const int pY = y & kMaxTableSizeMask;
            const int pZ = z & kMaxTableSizeMask;
            
            return kPermutation[kPermutation[kPermutation[pX] + pY] + pZ];
        }
    }

    PerlinNoise2D::PerlinNoise2D()
    {
        Seed();
    }

    PerlinNoise2D::PerlinNoise2D(const uint64_t seed)
    {
        Seed(seed);
    }
    
    void PerlinNoise2D::Seed()
    {
        m_rng.SeedFromTime();
        for (auto& m_gradient : m_gradients)
        {
            m_gradient = m_rng.RandUnitVector2();
        }
    }
    
    void PerlinNoise2D::Seed(const uint64_t seed)
    {
        m_rng.SetSeed(seed);
        for (auto& m_gradient : m_gradients)
        {
            m_gradient = m_rng.RandUnitVector2();
        }
    }
    
    uint64_t PerlinNoise2D::GetSeed() const
    {
        return m_rng.GetLastSeed();
    }
    
    float PerlinNoise2D::CalculateNoise(float x, float y, uint32_t noiseInputRange, int octaves, float persistence) const
    {
        float totalNoise = 0.f;
        float currentAmplitude = 1.f;
        float totalAmplitude = 0.f;

        for (int i = octaves - 1; i >= 0; --i)
        {
            totalAmplitude += currentAmplitude;
            const float noiseGridX = x * static_cast<float>(noiseInputRange);
            const float noiseGridY = y * static_cast<float>(noiseInputRange);

            const float localNoise = CalculateNoise(noiseGridX, noiseGridY);

            totalNoise += localNoise * currentAmplitude;
            currentAmplitude *= persistence;
            noiseInputRange *= 2;
        }

        totalNoise /= totalAmplitude;
        return totalNoise;
    }

    float PerlinNoise2D::CalculateNoise(float noiseX, float noiseY) const
    {
        int xi0 = math::FloorTo<int>(noiseX);
        int yi0 = math::FloorTo<int>(noiseY);

        const int xi1 = (xi0 + 1) & static_cast<int>(internal::kMaxTableSizeMask);
        const int yi1 = (yi0 + 1) & static_cast<int>(internal::kMaxTableSizeMask);

        const float deltaX = noiseX - static_cast<float>(xi0);
        const float deltaY = noiseY - static_cast<float>(yi0);

        xi0 &= internal::kMaxTableSizeMask;
        yi0 &= internal::kMaxTableSizeMask;

        // c00 == corner x0, y0
        const Vec2& c00 = m_gradients[internal::GetPermutationValue2D(xi0, yi0)];
        // c10 == corner x1, y0
        const Vec2& c10 = m_gradients[internal::GetPermutationValue2D(xi1, yi0)];
        // c01 == corner x0, y1
        const Vec2& c01 = m_gradients[internal::GetPermutationValue2D(xi0, yi1)];
        // c11 == corner x1, y1
        const Vec2& c11 = m_gradients[internal::GetPermutationValue2D(xi1, yi1)];

        const float x0 = deltaX;
        const float x1 = deltaX - 1.f;
        const float y0 = deltaY;
        const float y1 = deltaY - 1.f;

        // Vector2f p00 == to Corner 00 
        const Vec2 p00 = { x0, y0 };
        const Vec2 p10 = { x1, y0 };
        const Vec2 p01 = { x0, y1 };
        const Vec2 p11 = { x1, y1 };

        float smoothX = math::SmoothStep(deltaX);
        float smoothY = math::SmoothStep(deltaY);

        float v1 = Vec2::Dot(c00, p00);
        float v2 = Vec2::Dot(c10, p10);
        float v3 = Vec2::Dot(c01, p01);
        float v4 = Vec2::Dot(c11, p11);

        float resultX = math::Lerp(v1, v2, smoothX);
        float resultY = math::Lerp(v3, v4, smoothX);
        float result = math::Lerp(resultX, resultY, smoothY);

        // Normalize the Result:
        return (result + 1.f) * 0.5f;
    }


}