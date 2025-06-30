// UVec4Reg.inl
#pragma once

namespace nes
{
    UVec4Reg::UVec4Reg(const uint32 x, const uint32 y, const uint32 z, const uint32 w)
    {
#if defined (NES_USE_SSE)
        m_value = _mm_set_epi32(static_cast<int>(w), static_cast<int>(z), static_cast<int>(y), static_cast<int>(x));
#else
        m_u32[0] = x;
        m_u32[1] = y;
        m_u32[2] = z;
        m_u32[3] = w;
#endif
    }
    
    uint32& UVec4Reg::operator[](const size_t index)
    {
        NES_ASSERT(index < 4);
        return m_u32[index];
    }

    uint32 UVec4Reg::operator[](const size_t index) const
    {
        NES_ASSERT(index < 4);
        return m_u32[index];
    }

    UVec4Reg UVec4Reg::operator*(const UVec4Reg& other) const
    {
#if defined (NES_USE_SSE)
        return _mm_mullo_epi32(m_value, other.m_value);
#else
        UVec4Reg result;
        for (int i = 0; i < 4; ++i)
        {
            result.m_u32[i] = m_u32[i] * other.m_u32[i];
        }
        return result;
#endif
    }

    UVec4Reg& UVec4Reg::operator*=(const UVec4Reg& other)
    {
#if defined (NES_USE_SSE)
        m_value = _mm_mullo_epi32(m_value, other.m_value);
#else
        for (int i = 0; i < 4; ++i)
        {
            m_u32[i] *= other.m_u32[i];
        }
#endif
        return *this;
    }

    UVec4Reg UVec4Reg::operator+(const UVec4Reg& other) const
    {
    #if defined (NES_USE_SSE)
        return _mm_add_epi32(m_value, other.m_value);
    #else
        UVec4Reg result;
        for (int i = 0; i < 4; ++i)
        {
            result.m_u32[i] = m_u32[i] + other.m_u32[i];
        }
        return result;
    #endif
    }

    UVec4Reg& UVec4Reg::operator+=(const UVec4Reg& other)
    {
#if defined (NES_USE_SSE)
        m_value = _mm_add_epi32(m_value, other.m_value);
#else
        for (int i = 0; i < 4; ++i)
        {
            m_u32[i] += other.m_u32[i];
        }
#endif
        return *this;
    }

    uint32 UVec4Reg::GetX() const
    {
#if defined (NES_USE_SSE)
        return static_cast<uint32_t>(_mm_cvtsi128_si32(m_value));
#else
        return m_u32[0];
#endif
    }

    uint32 UVec4Reg::GetY() const
    {
        return m_u32[1];
    }

    uint32 UVec4Reg::GetZ() const
    {
        return m_u32[2];
    }

    uint32 UVec4Reg::GetW() const
    {
        return m_u32[3];
    }

    template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
    UVec4Reg UVec4Reg::Swizzle() const
    {
        static_assert(SwizzleX < 4, "SwizzleX value must be in range [0, 3]!");
        static_assert(SwizzleY < 4, "SwizzleY value must be in range [0, 3]!");
        static_assert(SwizzleZ < 4, "SwizzleZ value must be in range [0, 3]!");
        static_assert(SwizzleW < 4, "SwizzleW value must be in range [0, 3]!");

    #if defined (NES_USE_SSE)
        return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(SwizzleW, SwizzleZ, SwizzleY, SwizzleX));
    #else
        return UVec4Reg
        (
            m_u32[SwizzleX],
            m_u32[SwizzleY],
            m_u32[SwizzleZ],
            m_u32[SwizzleW]
        );
    #endif
    }

    int UVec4Reg::CountTrues() const
    {
    #if defined (NES_USE_SSE)
        return static_cast<int>(math::CountBits(_mm_movemask_ps(_mm_castsi128_ps(m_value))));
    #else
        return (m_u32[0] >> 31) + (m_u32[1] >> 31) + (m_u32[2] >> 31) + (m_u32[3] >> 31);
    #endif
    }

    int UVec4Reg::GetTrues() const
    {
    #if defined (NES_USE_SSE)
        return _mm_movemask_ps(_mm_castsi128_ps(m_value));
    #else
        return (m_u32[0] >> 31) | ((m_u32[1] >> 31) << 1) | ((m_u32[2] >> 31) << 2) | ((m_u32[3] >> 31) << 3);
    #endif
    }

    Vec4Reg UVec4Reg::ToFloat() const
    {
    #if defined (NES_USE_SSE)
        return _mm_cvtepi32_ps(m_value);
    #else
        return Vec4Reg
        (
            static_cast<float>(m_u32[0]),
            static_cast<float>(m_u32[1]),
            static_cast<float>(m_u32[2]),
            static_cast<float>(m_u32[3])
        );
    #endif
    }

    Vec4Reg UVec4Reg::ReinterpretAsFloat() const
    {
    #if defined (NES_USE_SSE)
        return Vec4Reg(_mm_castsi128_ps(m_value));
    #else
        return *reinterpret_cast<const Vec4Reg*>(this);
    #endif
    }

    inline void UVec4Reg::StoreInt4(uint32* pOutValues) const
    {
    #if defined (NES_USE_SSE)
        _mm_storeu_si128(reinterpret_cast<__m128i*>(pOutValues), m_value);
    #else
        for (int i = 0; i < 4; ++i)
        {
            pOutValues[i] = m_u32[i];
        }
    #endif
    }

    inline void UVec4Reg::StoreInt4Aligned(uint32* pOutValues) const
    {
    #if defined (NES_USE_SSE)
        _mm_store_si128(reinterpret_cast<__m128i*>(pOutValues), m_value);
    #else
        for (int i = 0; i < 4; ++i)
        {
            pOutValues[i] = m_u32[i];
        }
    #endif
    }

    UVec4Reg UVec4Reg::SplatX() const
    {
    #if defined (NES_USE_SSE)
        return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0));
    #else
        return UVec4Reg
        (
            m_u32[0],
            m_u32[0],
            m_u32[0],
            m_u32[0]
        );
    #endif
    }

    UVec4Reg UVec4Reg::SplatY() const
    {
    #if defined (NES_USE_SSE)
        return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(1, 1, 1, 1));
    #else
        return UVec4Reg
        (
            m_u32[1],
            m_u32[1],
            m_u32[1],
            m_u32[1]
        );
    #endif
    }

    UVec4Reg UVec4Reg::SplatZ() const
    {
    #if defined (NES_USE_SSE)
        return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(2, 2, 2, 2));
    #else
        return UVec4Reg
        (
            m_u32[2],
            m_u32[2],
            m_u32[2],
            m_u32[2]
        );
    #endif
    }

    UVec4Reg UVec4Reg::SplatW() const
    {
    #if defined (NES_USE_SSE)
        return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(3, 3, 3, 3));
    #else
        return UVec4Reg
        (
            m_u32[3],
            m_u32[3],
            m_u32[3],
            m_u32[3]
        );
    #endif
    }

    template <const uint Count>
    UVec4Reg UVec4Reg::LogicalShiftLeft() const
    {
        static_assert(Count <= 31, "Invalid shift");

    #if defined (NES_USE_SSE)
        return _mm_slli_epi32(m_value, Count);
    #else
        return UVec4Reg(m_u32[0] << Count, m_u32[1] << Count, m_u32[2] << Count, m_u32[3] << Count);
    #endif
    }

    template <const uint Count>
    UVec4Reg UVec4Reg::LogicalShiftRight() const
    {
        static_assert(Count <= 31, "Invalid shift");
        
    #if defined (NES_USE_SSE)
        return _mm_srli_epi32(m_value, Count);
    #else
        return UVec4Reg(m_u32[0] >> Count, m_u32[1] >> Count, m_u32[2] >> Count, m_u32[3] >> Count);
    #endif
    }

    template <const uint Count>
    UVec4Reg UVec4Reg::ArithmeticShiftRight() const
    {
        static_assert(Count <= 31, "Invalid shift");

    #if defined (NES_USE_SSE)
        return _mm_srai_epi32(m_value, Count);
    #else
        return UVec4Reg
        (
            uint32(int32_t(m_u32[0]) >> Count),
            uint32(int32_t(m_u32[1]) >> Count),
            uint32(int32_t(m_u32[2]) >> Count),
            uint32(int32_t(m_u32[3]) >> Count)
        );
    #endif
    }

    UVec4Reg UVec4Reg::ShiftComponents4Minus(const int count) const
    {
    #if defined (NES_USE_SSE4_1)
        alignas(UVec4Reg) static constexpr uint32 kFourMinusXShuffle[5][4] =
        {
            { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
            { 0x0f0e0d0c, 0xffffffff, 0xffffffff, 0xffffffff },
            { 0x0b0a0908, 0x0f0e0d0c, 0xffffffff, 0xffffffff },
            { 0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0xffffffff },
            { 0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c }
        };
    #endif

    #if defined(NES_USE_SSE4_1)
        return _mm_shuffle_epi8(m_value, *reinterpret_cast<const UVec4Reg::Type*>(kFourMinusXShuffle[count]));
    #else
        UVec4Reg result;
        for (int i = 0; i < 4; ++i)
        {
            result.m_u32[i] = m_u32[i + 4 - count];
        }
        return result;
    #endif
        
    }

    UVec4Reg UVec4Reg::Zero()
    {
    #if defined (NES_USE_SSE)
        return _mm_setzero_si128();
    #else
        return UVec4Reg(0, 0, 0, 0 );
    #endif
    }

    UVec4Reg UVec4Reg::Replicate(const uint32 value)
    {
    #if defined (NES_USE_SSE)
        return _mm_set1_epi32(static_cast<int>(value));
    #else
        return UVec4Reg
        (
            value, value, value, value
        );
    #endif
    }

    UVec4Reg UVec4Reg::LoadInt(const uint32* pValue)
    {
    #if defined (NES_USE_SSE)
        return _mm_castps_si128(_mm_load_ss(reinterpret_cast<const float*>(pValue)));
    #else
        return UVec4Reg
        (
            *pValues, 0, 0, 0
        );
    #endif
    }

    UVec4Reg UVec4Reg::LoadInt4(const uint32* pValues)
    {
    #if defined (NES_USE_SSE)
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(pValues));
    #else
        return UVec4Reg
        (
            pValues[0], pValues[1], pValues[2], pValues[3]
        );
    #endif
    }

    UVec4Reg UVec4Reg::LoadInt4Aligned(const uint32* pValues)
    {
    #if defined (NES_USE_SSE)
        return _mm_load_si128(reinterpret_cast<const __m128i*>(pValues));
    #else
        return UVec4Reg
        (
            pValues[0], pValues[1], pValues[2], pValues[3]
        );
    #endif
    }

    // void UVec4Reg::StoreInt4(const UVec4Reg& vec, uint32* pOutValues)
    // {
    // #if defined (NES_USE_SSE)
    //     _mm_storeu_si128(reinterpret_cast<__m128i *>(pOutValues), vec.m_value);
    // #else
    //     pOutValues[0] = vec.m_u32[0];
    //     pOutValues[1] = vec.m_u32[1];
    //     pOutValues[2] = vec.m_u32[2];
    //     pOutValues[3] = vec.m_u32[3];
    // #endif
    // }

    UVec4Reg UVec4Reg::Min(const UVec4Reg& a, const UVec4Reg& b)
    {
    #if defined (NES_USE_SSE)
        return _mm_min_epu32(a.m_value, b.m_value);
    #else
        return UVec4Reg
        (
            math::Min(a.m_u32[0], b.m_u32[0]),
            math::Min(a.m_u32[1], b.m_u32[1]),
            math::Min(a.m_u32[2], b.m_u32[2]),
            math::Min(a.m_u32[3], b.m_u32[3])
        );
    #endif
    }

    UVec4Reg UVec4Reg::Max(const UVec4Reg& a, const UVec4Reg& b)
    {
    #if defined (NES_USE_SSE)
        return _mm_max_epu32(a.m_value, b.m_value);
    #else
        return UVec4Reg
        (
            math::Max(a.m_u32[0], b.m_u32[0]),
            math::Max(a.m_u32[1], b.m_u32[1]),
            math::Max(a.m_u32[2], b.m_u32[2]),
            math::Max(a.m_u32[3], b.m_u32[3])
        );
    #endif
    }

    UVec4Reg UVec4Reg::Equals(const UVec4Reg& a, const UVec4Reg& b)
    {
    #if defined (NES_USE_SSE)
        return _mm_cmpeq_epi32(a.m_value, b.m_value);
    #else
        return UVec4Reg
        (
            a.m_u32[0] == b.m_u32[0] ? 0xffffffffu : 0,
            a.m_u32[1] == b.m_u32[1] ? 0xffffffffu : 0,
            a.m_u32[2] == b.m_u32[2] ? 0xffffffffu : 0,
            a.m_u32[3] == b.m_u32[3] ? 0xffffffffu : 0
        );
    #endif
    }

    UVec4Reg UVec4Reg::Or(const UVec4Reg& a, const UVec4Reg& b)
    {
    #if defined (NES_USE_SSE)
        return _mm_or_si128(a.m_value, b.m_value);
    #else
        return UVec4Reg
        (
            a.m_u32[0] | b.m_u32[0],
            a.m_u32[1] | b.m_u32[1],
            a.m_u32[2] | b.m_u32[2],
            a.m_u32[3] | b.m_u32[3]
        );
    #endif
    }

    UVec4Reg UVec4Reg::And(const UVec4Reg& a, const UVec4Reg& b)
    {
    #if defined (NES_USE_SSE)
        return _mm_and_si128(a.m_value, b.m_value);
    #else
        return UVec4Reg
        (
            a.m_u32[0] & b.m_u32[0],
            a.m_u32[1] & b.m_u32[1],
            a.m_u32[2] & b.m_u32[2],
            a.m_u32[3] & b.m_u32[3]
        );
    #endif
    }

    UVec4Reg UVec4Reg::Not(const UVec4Reg& value)
    {
    #if defined (NES_USE_SSE)
        return Xor(value, Replicate(0xffffffff));
    #else
        return UVec4Reg
        (
            ~value.m_u32[0],
            ~value.m_u32[1],
            ~value.m_u32[2],
            ~value.m_u32[3]
        );
    #endif
    }

    UVec4Reg UVec4Reg::Xor(const UVec4Reg& a, const UVec4Reg& b)
    {
    #if defined (NES_USE_SSE)
        return _mm_xor_si128(a.m_value, b.m_value);
    #else
        return UVec4Reg
        (
            a.m_u32[0] ^ b.m_u32[0],
            a.m_u32[1] ^ b.m_u32[1],
            a.m_u32[2] ^ b.m_u32[2],
            a.m_u32[3] ^ b.m_u32[3]
        );
    #endif
    }

    UVec4Reg UVec4Reg::Select(const UVec4Reg& notSet, const UVec4Reg& set, const UVec4Reg& mask)
    {
    #if defined (NES_USE_SSE)
        const __m128 isSet = _mm_castsi128_ps(_mm_srai_epi32(mask.m_value, 31));
        return _mm_castps_si128(_mm_or_ps(_mm_and_ps(isSet, _mm_castsi128_ps(set.m_value)), _mm_andnot_ps(isSet, _mm_castsi128_ps(notSet.m_value))));
    #else
        UVec4Reg result;
        for (int i = 0; i < 4; ++i)
        {
            result.m_u32[i] = (mask.m_u32[i] & 0x80000000u) ? set.m_u32[i] : notSet.m_u32[i];
        }        
        return result;
    #endif
    }

    UVec4Reg UVec4Reg::Sort4True(const UVec4Reg& value, const UVec4Reg& index)
    {
        // If value.z is false, then shift W to Z.
        UVec4Reg result = UVec4Reg::Select(index.Swizzle<ESwizzleX, ESwizzleY, ESwizzleW, ESwizzleW>(), index, value.SplatZ());

        // If value.y is false, then shift Z and further to Y and further.
        result = UVec4Reg::Select(result.Swizzle<ESwizzleX, ESwizzleZ, ESwizzleW, ESwizzleW>(), result, value.SplatY());

        // If value.x is false, then shift X and further to Y and further.
        result = UVec4Reg::Select(result.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleW, ESwizzleW>(), result, value.SplatX());

        return result;
    }

    int UVec4Reg::CountAndSortTrues(const UVec4Reg& value, UVec4Reg& identifiers)
    {
        identifiers = UVec4Reg::Sort4True(value, identifiers);
        return value.CountTrues();
    }  
}