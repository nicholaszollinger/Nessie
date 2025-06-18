// UVec4.inl
#pragma once

// namespace nes
// {
//     UVec4::UVec4(const uint32 x, const uint32 y, const uint32 z, const uint32 w)
//         : x(x)
//         , y(y)
//         , z(z)
//         , w(w)
//     {
//         //
//     }
//
//     bool UVec4::operator==(const UVec4& other) const
//     {
//         return Equals(UVec4Reg::LoadUVec4(this), other).TestAllTrue();
//     }
//
//     uint32& UVec4::operator[](const size_t index)
//     {
//         NES_ASSERT(index < 4);
//         return *(&x + index);
//     }
//
//     uint32 UVec4::operator[](const size_t index) const
//     {
//         NES_ASSERT(index < 4);
//         return *(&x + index);
//     }
//
//     UVec4 UVec4::operator*(const UVec4& other) const
//     {
//         
//     }
//
//     UVec4& UVec4::operator*=(const UVec4& other)
//     {
//     #if defined(NES_USE_SSE)
//         m_value = _mm_mullo_epi32(m_value, other.m_value);
//     #else
//         for (int i = 0; i < 4; ++i)
//         {
//             m_u32[i] *= other.m_u32[i];
//         }
//     #endif
//         return *this;
//     }
//
//     UVec4 UVec4::operator+(const UVec4& other) const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_add_epi32(m_value, other.m_value);
//     #else
//         UVec4 result;
//         for (int i = 0; i < 4; ++i)
//         {
//             result.m_u32[i] = m_u32[i] + other.m_u32[i];
//         }
//         return result;
//     #endif
//     }
//
//     UVec4& UVec4::operator+=(const UVec4& other)
//     {
//     #if defined(NES_USE_SSE)
//         m_value = _mm_add_epi32(m_value, other.m_value);
//     #else
//         for (int i = 0; i < 4; ++i)
//         {
//             m_u32[i] += other.m_u32[i];
//         }
//     #endif
//         return *this;
//     }
//
//     uint32 UVec4::GetX() const
//     {
//     #if defined(NES_USE_SSE)
//         return static_cast<uint32_t>(_mm_cvtsi128_si32(m_value));
//     #else
//         return m_u32[0];
//     #endif
//     }
//
//     uint32 UVec4::GetY() const
//     {
//         return m_u32[1];
//     }
//
//     uint32 UVec4::GetZ() const
//     {
//         return m_u32[2];
//     }
//
//     uint32 UVec4::GetW() const
//     {
//         return m_u32[3];
//     }
//
//     template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
//     UVec4 UVec4::Swizzle() const
//     {
//         static_assert(SwizzleX < 4, "SwizzleX value must be in range [0, 3]!");
//         static_assert(SwizzleY < 4, "SwizzleY value must be in range [0, 3]!");
//         static_assert(SwizzleZ < 4, "SwizzleZ value must be in range [0, 3]!");
//         static_assert(SwizzleW < 4, "SwizzleW value must be in range [0, 3]!");
//
//     #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(SwizzleW, SwizzleZ, SwizzleY, SwizzleX));
//     #else
//         return UVec4
//         (
//             m_u32[SwizzleX],
//             m_u32[SwizzleY],
//             m_u32[SwizzleZ],
//             m_u32[SwizzleW]
//         );
//     #endif
//     }
//
//     int UVec4::CountTrues() const
//     {
//     #if defined(NES_USE_SSE)
//         return static_cast<int>(math::CountBits(_mm_movemask_ps(_mm_castsi128_ps(m_value))));
//     #else
//         return (m_u32[0] >> 31) + (m_u32[1] >> 31) + (m_u32[2] >> 31) + (m_u32[3] >> 31);
//     #endif
//     }
//
//     int UVec4::GetTrues() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_movemask_ps(_mm_castsi128_ps(m_value));
//     #else
//         return (m_u32[0] >> 31) | ((m_u32[1] >> 31) << 1) | ((m_u32[2] >> 31) << 2) | ((m_u32[3] >> 31) << 3);
//     #endif
//     }
//
//     bool UVec4::TestAllTrue() const
//     {
//         return GetTrues() == 0b1111;
//     }
//
//     bool UVec4::TestAnyXYZTrue() const
//     {
//         return (GetTrues() & 0b111) != 0;
//     }
//
//     bool UVec4::TestAllXYZTrue() const
//     {
//         return (GetTrues() & 0b111) == 0b111;
//     }
//
//     Vec4 UVec4::ToFloat() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_cvtepi32_ps(m_value);
//     #else
//         return UVec4
//         (
//             static_cast<float>(m_u32[0]),
//             static_cast<float>(m_u32[1]),
//             static_cast<float>(m_u32[2]),
//             static_cast<float>(m_u32[3])
//         );
//     #endif
//     }
//
//     Vec4 UVec4::ReinterpretAsFloat() const
//     {
//     #if defined(NES_USE_SSE)
//         return Vec4(_mm_castsi128_ps(m_value));
//     #else
//         return *reinterpret_cast<const Vec4*>(this);
//     #endif
//     }
//
//     UVec4 UVec4::SplatX() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0));
//     #else
//         return UVec4
//         (
//             m_u32[0],
//             m_u32[0],
//             m_u32[0],
//             m_u32[0]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::SplatY() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0));
//     #else
//         return UVec4
//         (
//             m_u32[1],
//             m_u32[1],
//             m_u32[1],
//             m_u32[1]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::SplatZ() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0));
//     #else
//         return UVec4
//         (
//             m_u32[2],
//             m_u32[2],
//             m_u32[2],
//             m_u32[2]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::SplatW() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0));
//     #else
//         return UVec4
//         (
//             m_u32[3],
//             m_u32[3],
//             m_u32[3],
//             m_u32[3]
//         );
//     #endif
//     }
//
//     template <const uint Count>
//     UVec4 UVec4::LogicalShiftLeft() const
//     {
//         static_assert(Count <= 31, "Invalid shift");
//     #if defined(NES_USE_SSE)
//         return _mm_slli_epi32(m_value, Count);
//     #else
//         return UVec4(m_u32[0] << Count, m_u32[1] << Count, m_u32[2] << Count, m_u32[3] << Count);
//     #endif
//     }
//
//     template <const uint Count>
//     UVec4 UVec4::LogicalShiftRight() const
//     {
//         static_assert(Count <= 31, "Invalid shift");
//     #if defined(NES_USE_SSE)
//         return _mm_srli_epi32(m_value, Count);
//     #else
//         return UVec4(m_u32[0] >> Count, m_u32[1] >> Count, m_u32[2] >> Count, m_u32[3] >> Count);
//     #endif
//     }
//
//     template <const uint Count>
//     UVec4 UVec4::ArithmeticShiftRight() const
//     {
//         static_assert(Count <= 31, "Invalid shift");
//
//     #if defined(NES_USE_SSE)
//         return _mm_srai_epi32(m_value, Count);
//     #else
//         return UVec4
//         (
//             uint32(int32_t(m_u32[0]) >> Count),
//             uint32(int32_t(m_u32[1]) >> Count),
//             uint32(int32_t(m_u32[2]) >> Count),
//             uint32(int32_t(m_u32[3]) >> Count)
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Zero()
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_setzero_si128();
//     #else
//         return UVec4(0, 0, 0, 0 );
//     #endif
//     }
//
//     UVec4 UVec4::Replicate(const uint32 value)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_set1_epi32(static_cast<int>(value));
//     #else
//         return UVec4
//         (
//             value, value, value, value
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Load(const uint32* pValues)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_loadu_si128(reinterpret_cast<const __m128i*>(pValues));
//     #else
//         return UVec4
//         (
//             pValues[0], pValues[1], pValues[2], pValues[3]
//         );
//     #endif
//     }
//
//     void UVec4::Store(const UVec4& vec, uint32* pOutValues)
//     {
//     #if defined(NES_USE_SSE)
//         _mm_storeu_si128(reinterpret_cast<__m128i *>(pOutValues), vec.m_value);
//     #else
//         pOutValues[0] = vec.m_u32[0];
//         pOutValues[1] = vec.m_u32[1];
//         pOutValues[2] = vec.m_u32[2];
//         pOutValues[3] = vec.m_u32[3];
//     #endif
//     }
//
//     UVec4 UVec4::Min(const UVec4& a, const UVec4& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_min_epu32(a.m_value, b.m_value);
//     #else
//         return UVec4
//         (
//             math::Min(a.m_u32[0], b.m_u32[0]),
//             math::Min(a.m_u32[1], b.m_u32[1]),
//             math::Min(a.m_u32[2], b.m_u32[2]),
//             math::Min(a.m_u32[3], b.m_u32[3])
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Max(const UVec4& a, const UVec4& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_max_epu32(a.m_value, b.m_value);
//     #else
//         return UVec4
//         (
//             math::Max(a.m_u32[0], b.m_u32[0]),
//             math::Max(a.m_u32[1], b.m_u32[1]),
//             math::Max(a.m_u32[2], b.m_u32[2]),
//             math::Max(a.m_u32[3], b.m_u32[3])
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Equals(const UVec4& a, const UVec4& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_cmpeq_epi32(a.m_value, b.m_value);
//     #else
//         return UVec4
//         (
//             a.m_u32[0] == b.m_u32[0] ? 0xffffffffu : 0,
//             a.m_u32[1] == b.m_u32[1] ? 0xffffffffu : 0,
//             a.m_u32[2] == b.m_u32[2] ? 0xffffffffu : 0,
//             a.m_u32[3] == b.m_u32[3] ? 0xffffffffu : 0
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Or(const UVec4& a, const UVec4& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_or_si128(a.m_value, b.m_value);
//     #else
//         return UVec4
//         (
//             a.m_u32[0] | b.m_u32[0],
//             a.m_u32[1] | b.m_u32[1],
//             a.m_u32[2] | b.m_u32[2],
//             a.m_u32[3] | b.m_u32[3]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::And(const UVec4& a, const UVec4& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_and_si128(a.m_value, b.m_value);
//     #else
//         return UVec4
//         (
//             a.m_u32[0] & b.m_u32[0],
//             a.m_u32[1] & b.m_u32[1],
//             a.m_u32[2] & b.m_u32[2],
//             a.m_u32[3] & b.m_u32[3]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Not(const UVec4& value)
//     {
//     #if defined(NES_USE_SSE)
//         return Xor(value, Replicate(0xffffffff));
//     #else
//         return UVec4
//         (
//             ~value.m_u32[0],
//             ~value.m_u32[1],
//             ~value.m_u32[2],
//             ~value.m_u32[3]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Xor(const UVec4& a, const UVec4& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_xor_si128(a.m_value, b.m_value);
//     #else
//         return UVec4
//         (
//             a.m_u32[0] ^ b.m_u32[0],
//             a.m_u32[1] ^ b.m_u32[1],
//             a.m_u32[2] ^ b.m_u32[2],
//             a.m_u32[3] ^ b.m_u32[3]
//         );
//     #endif
//     }
//
//     UVec4 UVec4::Select(const UVec4& notSet, const UVec4& set, const UVec4& mask)
//     {
//     #if defined(NES_USE_SSE)
//         const __m128 isSet = _mm_castsi128_ps(_mm_srai_epi32(mask.m_value, 31));
//         return _mm_castps_si128(_mm_or_ps(_mm_and_ps(isSet, _mm_castsi128_ps(set.m_value)), _mm_andnot_ps(isSet, _mm_castsi128_ps(notSet.m_value))));
//     #else
//         UVec4 result;
//         for (int i = 0; i < 4; ++i)
//         {
//             result.m_u32[i] = (mask.m_u32[i] & 0x80000000u) ? set.m_u32[i] : notSet.m_u32[i];
//         }        
//         return result;
//     #endif
//     }
//
//     UVec4 UVec4::SortTrue(const UVec4& value, const UVec4& index)
//     {
//         // If value.z is false then shift W to Z.
//         UVec4 result = UVec4::Select(index.Swizzle<0, 1, 3, 3>(), index, value.SplatZ());
//
//         // If value.y is false then shift Z and further to Y and further.
//         result = UVec4::Select(result.Swizzle<0, 2, 3, 3>(), result, value.SplatY());
//
//         // If value.x is false then shift X and further to Y and further.
//         result = UVec4::Select(value.Swizzle<1, 2, 3, 3>(), result, value.SplatX());
//
//         return result;
//     }
//
//     int UVec4::CountAndSortTrues(const UVec4& value, UVec4& identifiers)
//     {
//         identifiers = UVec4::SortTrue(value, identifiers);
//         return value.CountTrues();
//     }   
// }
