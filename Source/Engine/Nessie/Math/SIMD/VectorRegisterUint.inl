// // VectorRegisterUint.inl
//
// #include "Math/VectorRegister.h"
//
// namespace nes
// {
//     VectorRegisterUint::VectorRegisterUint(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t w)
//     {
//     #if defined(NES_USE_SSE)
//         m_value = _mm_set_epi32(static_cast<int>(w), static_cast<int>(z), static_cast<int>(y), static_cast<int>(x));
//     #else
//         m_u32[0] = x;
//         m_u32[1] = y;
//         m_u32[2] = z;
//         m_u32[3] = w;
//     #endif
//     }
//
//     bool VectorRegisterUint::operator==(const VectorRegisterUint& other) const
//     {
//         return Equals(*this, other).TestAllTrue();
//     }
//
//     uint32_t& VectorRegisterUint::operator[](const uint32_t index)
//     {
//         NES_ASSERT(index < 4);
//         return m_u32[index];
//     }
//
//     uint32_t VectorRegisterUint::operator[](const uint32_t index) const
//     {
//         NES_ASSERT(index < 4);
//         return m_u32[index];
//     }
//
//     uint32_t VectorRegisterUint::GetX() const
//     {
//     #if defined(NES_USE_SSE)
//             return static_cast<uint32_t>(_mm_cvtsi128_si32(m_value));
//     #else
//             return m_u32[0];
//     #endif
//     }
//
//     uint32_t VectorRegisterUint::GetY() const
//     {
//         return m_u32[1];
//     }
//
//     uint32_t VectorRegisterUint::GetZ() const
//     {
//         return m_u32[2];
//     }
//
//     uint32_t VectorRegisterUint::GetW() const
//     {
//         return m_u32[3];
//     }
//
//     template <uint32_t SwizzleX, uint32_t SwizzleY, uint32_t SwizzleZ, uint32_t SwizzleW>
//     VectorRegisterUint VectorRegisterUint::Swizzle() const
//     {
//         static_assert(SwizzleX < 4, "SwizzleX value must be in range [0, 3]!");
//         static_assert(SwizzleY < 4, "SwizzleY value must be in range [0, 3]!");
//         static_assert(SwizzleZ < 4, "SwizzleZ value must be in range [0, 3]!");
//         static_assert(SwizzleW < 4, "SwizzleW value must be in range [0, 3]!");
//
// #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(SwizzleW, SwizzleZ, SwizzleY, SwizzleX));
// #else
//         return VectorRegisterUint
//         (
//             m_u32[SwizzleX],
//             m_u32[SwizzleY],
//             m_u32[SwizzleZ],
//             m_u32[SwizzleW]
//         );
// #endif
//     }
//
//     int VectorRegisterUint::CountTrues() const
//     {
//     #if defined(NES_USE_SSE)
//             return static_cast<int>(math::CountBits(_mm_movemask_ps(_mm_castsi128_ps(m_value))));
//     #else
//             return (m_u32[0] >> 31) + (m_u32[1] >> 31) + (m_u32[2] >> 31) + (m_u32[3] >> 31);
//     #endif
//     }
//
//     int VectorRegisterUint::GetTrues() const
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_movemask_ps(_mm_castsi128_ps(m_value));
//     #else
//         return (m_u32[0] >> 31) | ((m_u32[1] >> 31) << 1) | ((m_u32[2] >> 31) << 2) | ((m_u32[3] >> 31) << 3);
//     #endif
//     }
//
//     bool VectorRegisterUint::TestAllTrue() const
//     {
//         return GetTrues() == 0b1111;
//     }
//
//     inline bool VectorRegisterUint::TestAnyXYZTrue() const
//     {
//         return (GetTrues() & 0b111) != 0;
//     }
//
//     inline bool VectorRegisterUint::TestAllXYZTrue() const
//     {
//         return (GetTrues() & 0b111) == 0b111;
//     }
//
//     VectorRegisterF VectorRegisterUint::ConvertToFloat() const
//     {
// #if defined(NES_USE_SSE)
//         return _mm_cvtepi32_ps(m_value);
// #else
//         return VectorRegisterF
//         (
//             static_cast<float>(m_u32[0]),
//             static_cast<float>(m_u32[1]),
//             static_cast<float>(m_u32[2]),
//             static_cast<float>(m_u32[3]),
//         );
// #endif
//     }
//     
//     VectorRegisterF VectorRegisterUint::ReinterpretAsFloat() const
//     {
// #if defined(NES_USE_SSE)
//         return VectorRegisterF(_mm_castsi128_ps(m_value));
// #else
//         return *reinterpret_cast<const VectorRegisterF*>(this);
// #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::SplatX() const
//     {
// #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0));
// #else
//         return VectorRegisterUint
//         (
//             m_u32[0],
//             m_u32[0],
//             m_u32[0],
//             m_u32[0]
//         );
// #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::SplatY() const
//     {
// #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(1, 1, 1, 1));
// #else
//         return VectorRegisterUint
//         (
//             m_u32[1],
//             m_u32[1],
//             m_u32[1],
//             m_u32[1]
//         );
// #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::SplatZ() const
//     {
// #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(2, 2, 2, 2));
// #else
//         return VectorRegisterUint
//         (
//             m_u32[2],
//             m_u32[2],
//             m_u32[2],
//             m_u32[2]
//         );
// #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::SplatW() const
//     {
// #if defined(NES_USE_SSE)
//         return _mm_shuffle_epi32(m_value, _MM_SHUFFLE(3, 3, 3, 3));
// #else
//         return VectorRegisterUint
//         (
//             m_u32[3],
//             m_u32[3],
//             m_u32[3],
//             m_u32[3]
//         );
// #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Zero()
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_setzero_si128();
//     #else
//         return VectorRegisterUint
//         (
//             0, 0, 0, 0
//         );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Replicate(const uint32_t value)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_set1_epi32(static_cast<int>(value));
//     #else
//         return VectorRegisterF
//         (
//             value, value, value, value
//         );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Load(const uint32_t* pValues)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_loadu_si128(reinterpret_cast<const __m128i*>(pValues));
//     #else
//         return VectorRegisterUint
//         (
//             pValues[0], pValues[1], pValues[2], pValues[3]
//         );
//     #endif
//     }
//     
//     void VectorRegisterUint::Store(const VectorRegisterUint& vec, uint32_t* pOutValues)
//     {
//     #if defined(NES_USE_SSE)
//             _mm_storeu_si128(reinterpret_cast<__m128i *>(pOutValues), vec.m_value);
//     #else
//             pOutValues[0] = vec.m_u32[0];
//             pOutValues[1] = vec.m_u32[1];
//             pOutValues[2] = vec.m_u32[2];
//             pOutValues[3] = vec.m_u32[3];
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Min(const VectorRegisterUint& a, const VectorRegisterUint& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_min_epu32(a.m_value, b.m_value);
//     #else
//         return VectorRegisterUint
//         (
//             math::Min(a.m_u32[0], b.m_u32[0]),
//             math::Min(a.m_u32[1], b.m_u32[1]),
//             math::Min(a.m_u32[2], b.m_u32[2]),
//             math::Min(a.m_u32[3], b.m_u32[3]),
//         );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Max(const VectorRegisterUint& a, const VectorRegisterUint& b)
//     {
//     #if defined(NES_USE_SSE)
//             return _mm_max_epu32(a.m_value, b.m_value);
//     #else
//             return VectorRegisterUint
//             (
//                 math::Max(a.m_u32[0], b.m_u32[0]),
//                 math::Max(a.m_u32[1], b.m_u32[1]),
//                 math::Max(a.m_u32[2], b.m_u32[2]),
//                 math::Max(a.m_u32[3], b.m_u32[3]),
//             );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Equals(const VectorRegisterUint& a, const VectorRegisterUint& b)
//     {
//     #if defined(NES_USE_SSE)
//             return _mm_cmpeq_epi32(a.m_value, b.m_value);
//     #else
//             return VectorRegisterUint
//             (
//                 a.m_u32[0] == b.m_u32[0] ? 0xffffffffu : 0,
//                 a.m_u32[1] == b.m_u32[1] ? 0xffffffffu : 0,
//                 a.m_u32[2] == b.m_u32[2] ? 0xffffffffu : 0,
//                 a.m_u32[3] == b.m_u32[3] ? 0xffffffffu : 0
//             );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Or(const VectorRegisterUint& a, const VectorRegisterUint& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_or_si128(a.m_value, b.m_value);
//     #else
//         return VectorRegisterUint
//         (
//             a.m_u32[0] | b.m_u32[0],
//             a.m_u32[1] | b.m_u32[1],
//             a.m_u32[2] | b.m_u32[2],
//             a.m_u32[3] | b.m_u32[3]
//         );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::And(const VectorRegisterUint& a, const VectorRegisterUint& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_and_si128(a.m_value, b.m_value);
//     #else
//         return VectorRegisterUint
//         (
//             a.m_u32[0] & b.m_u32[0],
//             a.m_u32[1] & b.m_u32[1],
//             a.m_u32[2] & b.m_u32[2],
//             a.m_u32[3] & b.m_u32[3]
//         );
//     #endif
//     }
//     
//     VectorRegisterUint VectorRegisterUint::Xor(const VectorRegisterUint& a, const VectorRegisterUint& b)
//     {
//     #if defined(NES_USE_SSE)
//         return _mm_xor_si128(a.m_value, b.m_value);
//     #else
//         return VectorRegisterUint
//         (
//             a.m_u32[0] ^ b.m_u32[0],
//             a.m_u32[1] ^ b.m_u32[1],
//             a.m_u32[2] ^ b.m_u32[2],
//             a.m_u32[3] ^ b.m_u32[3]
//         );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Not(const VectorRegisterUint& value)
//     {
//     #if defined(NES_USE_SSE)
//         return Xor(value, Replicate(0xffffffff));
//     #else
//         return VectorRegisterUint
//         (
//             ~value.m_u32[0],
//             ~value.m_u32[1],
//             ~value.m_u32[2],
//             ~value.m_u32[3]
//         );
//     #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::Select(const VectorRegisterUint& notSet, const VectorRegisterUint& set, const VectorRegisterUint& control)
//     {
// #if defined(NES_USE_SSE)
//         const __m128 isSet = _mm_castsi128_ps(_mm_srai_epi32(control.m_value, 31));
//         return _mm_castps_si128(_mm_or_ps(_mm_and_ps(isSet, _mm_castsi128_ps(set.m_value)), _mm_andnot_ps(isSet, _mm_castsi128_ps(notSet.m_value))));
// #else
//         VectorRegisterUint result;
//         for (int i = 0; i < 4; ++i)
//         {
//             result.m_u32[i] = (control.m_u32[i] & 0x80000000u) ? set.m_u32[i] : notSet.m_u32[i];
//         }        
//         return result;
// #endif
//     }
//
//     VectorRegisterUint VectorRegisterUint::SortTrue(const VectorRegisterUint& value, const VectorRegisterUint& index)
//     {
//         // If value.z is false then shift W to Z.
//         VectorRegisterUint result = VectorRegisterUint::Select(index.Swizzle<0, 1, 3, 3>(), index, value.SplatZ());
//
//         // If value.y is false then shift Z and further to Y and further.
//         result = VectorRegisterUint::Select(result.Swizzle<0, 2, 3, 3>(), result, value.SplatY());
//
//         // If value.x is false then shift X and further to Y and further.
//         result = VectorRegisterUint::Select(value.Swizzle<1, 2, 3, 3>(), result, value.SplatX());
//
//         return result;
//     }
//
//     int VectorRegisterUint::CountAndSortTrues(const VectorRegisterUint& value, VectorRegisterUint& identifiers)
//     {
//         identifiers = VectorRegisterUint::SortTrue(value, identifiers);
//         return value.CountTrues();
//     }
// }
