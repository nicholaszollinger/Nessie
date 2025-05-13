// VectorRegisterF.inl
#pragma once
#include "VectorRegisterUint.h"

namespace nes
{
    VectorRegisterF::VectorRegisterF(const float x, const float y, const float z, const float w)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_set_ps(w, z, y, x);
    #else
        m_f32[0] = x;
        m_f32[1] = y;
        m_f32[2] = z;
        m_f32[3] = w;
    #endif
    }

    bool VectorRegisterF::operator==(const VectorRegisterF& other) const
    {
        return Equals(*this, other).TestAllTrue();
    }

    VectorRegisterF VectorRegisterF::operator+(const VectorRegisterF& other) const
    {
    #if defined(NES_USE_SSE)
        return _mm_add_ps(m_value, other.m_value);
    #else
        return VectorRegisterF
        (
            m_f32[0] + other.m_f32[0],
            m_f32[1] + other.m_f32[1],
            m_f32[2] + other.m_f32[2],
            m_f32[3] + other.m_f32[3]
        );
    #endif
    }

    VectorRegisterF& VectorRegisterF::operator+=(const VectorRegisterF& other)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_add_ps(m_value, other.m_value);
    #else
        m_f32[0] += other.m_f32[0];
        m_f32[1] += other.m_f32[1];
        m_f32[2] += other.m_f32[2];
        m_f32[3] += other.m_f32[3];
    #endif
        return *this;
    }

    VectorRegisterF VectorRegisterF::operator-(const VectorRegisterF& other) const
    {
#if defined(NES_USE_SSE)
        return _mm_sub_ps(m_value, other.m_value);
#else
        return VectorRegisterF
        (
            m_f32[0] - other.m_f32[0],
            m_f32[1] - other.m_f32[1],
            m_f32[2] - other.m_f32[2],
            m_f32[3] - other.m_f32[3]
        );
#endif
    }

    VectorRegisterF& VectorRegisterF::operator-=(const VectorRegisterF& other)
    {
#if defined(NES_USE_SSE)
        m_value = _mm_sub_ps(m_value, other.m_value);
#else
        m_f32[0] -= other.m_f32[0];
        m_f32[1] -= other.m_f32[1];
        m_f32[2] -= other.m_f32[2];
        m_f32[3] -= other.m_f32[3];
#endif
        return *this;
    }

    VectorRegisterF VectorRegisterF::operator*(const float value) const
    {
    #if defined(NES_USE_SSE)
        return _mm_mul_ps(m_value, _mm_set1_ps(value));
    #else
        return VectorRegisterF
        (
            m_f32[0] * value,
            m_f32[1] * value,
            m_f32[2] * value,
            m_f32[3] * value
        );
    #endif
    }

    VectorRegisterF& VectorRegisterF::operator*=(const float value)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_add_ps(m_value, _mm_set1_ps(value));
    #else
        m_f32[0] *= value;
        m_f32[1] *= value;
        m_f32[2] *= value;
        m_f32[3] *= value;
    #endif

        return *this;
    }

    VectorRegisterF operator*(const float value, const VectorRegisterF& vec)
    {
#if defined(NES_USE_SSE)
        return _mm_mul_ps(_mm_set1_ps(value), vec.m_value);
#else
        return VectorRegisterF(value * vec.m_f32[0], value * vec.m_f32[1], value * vec.m_f32[2]);
#endif
    }

    VectorRegisterF VectorRegisterF::operator*(const VectorRegisterF other) const
    {
    #if defined(NES_USE_SSE)
        return _mm_mul_ps(m_value, other.m_value);
    #else
        return VectorRegisterF
        (
            m_f32[0] * other.m_f32[0],
            m_f32[1] * other.m_f32[1],
            m_f32[2] * other.m_f32[2],
            m_f32[3] * other.m_f32[3]
        );
    #endif
    }

    VectorRegisterF& VectorRegisterF::operator*=(const VectorRegisterF other)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_mul_ps(m_value, other.m_value);
    #else
         m_f32[0] * other.m_f32[0];
         m_f32[1] * other.m_f32[1];
         m_f32[2] * other.m_f32[2];
         m_f32[3] * other.m_f32[3];
    #endif
        return *this;
    }

    inline VectorRegisterF VectorRegisterF::operator/(const VectorRegisterF other) const
    {
#if defined(NES_USE_SSE)
        return _mm_div_ps(m_value, other.m_value);
#else
        return VectorRegisterF
        (
            m_f32[0] / other.m_f32[0],
            m_f32[1] / other.m_f32[1],
            m_f32[2] / other.m_f32[2],
            m_f32[3] / other.m_f32[3]
        );
#endif
    }

    inline VectorRegisterF& VectorRegisterF::operator/=(const VectorRegisterF other)
    {
#if defined(NES_USE_SSE)
        m_value = _mm_div_ps(m_value, other.m_value);
#else
        m_f32[0] / other.m_f32[0];
        m_f32[1] / other.m_f32[1];
        m_f32[2] / other.m_f32[2];
        m_f32[3] / other.m_f32[3];
#endif
        return *this;
    }

    float& VectorRegisterF::operator[](const uint32_t index)
    {
        NES_ASSERT(index < 4);
        return m_f32[index];
    }

    float VectorRegisterF::operator[](const uint32_t index) const
    {
        NES_ASSERT(index < 4);
        return m_f32[index];
    }

    float VectorRegisterF::GetX() const
    {
    #if defined(NES_USE_SSE)
        return _mm_cvtss_f32(m_value);
    #else
        return m_f32[0];
    #endif
    }

    float VectorRegisterF::GetY() const
    {
        return m_f32[1];
    }

    float VectorRegisterF::GetZ() const
    {
        return m_f32[2];
    }

    float VectorRegisterF::GetW() const
    {
        return m_f32[3];
    }

    template <uint32_t SwizzleX, uint32_t SwizzleY, uint32_t SwizzleZ, uint32_t SwizzleW>
    VectorRegisterF VectorRegisterF::Swizzle() const
    {
        static_assert(SwizzleX < 4, "SwizzleX value must be in range [0, 3]!");
        static_assert(SwizzleY < 4, "SwizzleY value must be in range [0, 3]!");
        static_assert(SwizzleZ < 4, "SwizzleZ value must be in range [0, 3]!");
        static_assert(SwizzleW < 4, "SwizzleW value must be in range [0, 3]!");

#if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(SwizzleW, SwizzleZ, SwizzleY, SwizzleX));
#else
        return VectorRegister4
        (
            m_f32[SwizzleX],
            m_f32[SwizzleY],
            m_f32[SwizzleZ],
            m_f32[SwizzleW]
        );
#endif
    }

    inline bool VectorRegisterF::IsNaN() const
    {
#if defined(NES_USE_SSE)
        return (_mm_movemask_ps(_mm_cmpunord_ps(m_value, m_value)) & 0x7) != 0;
#else
        return std::isnan(m_f32[0]) || std::isnan(m_f32[1]) || std::isnan(m_f32[2]);
#endif
    }

    inline VectorRegisterF VectorRegisterF::Cross(const VectorRegisterF& other) const
    {
#if defined(NES_USE_SSE)
        Type t1 = _mm_shuffle_ps(other.m_value, other.m_value, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same.
        t1 = _mm_mul_ps(t1, m_value);
        Type t2 = _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same.
        t2 = _mm_mul_ps(t2, other.m_value);
        const Type t3 = _mm_sub_ps(t1, t2);
        return _mm_shuffle_ps(t3, t3, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same.
#else
        return VectorRegisterF(m_f32[1] * other.m_f32[2] - m_f32[2] * other.m_f32[1],
                m_f32[2] * other.m_f32[0] - m_f32[0] * other.m_f32[2],
                m_f32[0] * other.m_f32[1] - m_f32[1] * other.m_f32[0]);
#endif
    }

    inline float VectorRegisterF::Length() const
    {
#if defined(NES_USE_SSE)
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(m_value, m_value, 0x7f)));
#else
        return std::sqrt(SquaredLength());
#endif
    }

    inline float VectorRegisterF::SquaredLength() const
    {
#if defined(NES_USE_SSE)
        return _mm_cvtss_f32(_mm_dp_ps(m_value, m_value, 0x7f));
#else
        return (m_f32[0] * m_f32[0]) + (m_f32[1] * m_f32[1]) + (m_f32[2] * m_f32[2]);
#endif
    }

    VectorRegisterUint VectorRegisterF::ConvertToInt() const
    {
#if defined(NES_USE_SSE)
        return _mm_cvttps_epi32(m_value);
#else
        return VectorRegisterUint
        (
            static_cast<uint32_t>(m_f32[0]),
            static_cast<uint32_t>(m_f32[1]),
            static_cast<uint32_t>(m_f32[2]),
            static_cast<uint32_t>(m_f32[3]),
        );
#endif
    }

    VectorRegisterUint VectorRegisterF::ReinterpretAsInt() const
    {
#if defined(NES_USE_SSE)
        return VectorRegisterUint(_mm_castps_si128(m_value));
#else
        return *reinterpret_cast<const VectorRegisterUint*>(this);
#endif
    }

    VectorRegisterF VectorRegisterF::SplatX() const
    {
    #if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(0, 0, 0, 0));
    #else
        return VectorRegisterF
        (
            m_f32[0],
            m_f32[0],
            m_f32[0],
            m_f32[0]
        );
    #endif
    }

    VectorRegisterF VectorRegisterF::SplatY() const
    {
#if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(1, 1, 1, 1));
#else
        return VectorRegisterF
        (
            m_f32[1],
            m_f32[1],
            m_f32[1],
            m_f32[1]
        );
#endif
    }

    VectorRegisterF VectorRegisterF::SplatZ() const
    {
#if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(2, 2, 2, 2));
#else
        return VectorRegisterF
        (
            m_f32[2],
            m_f32[2],
            m_f32[2],
            m_f32[2]
        );
#endif
    }

    VectorRegisterF VectorRegisterF::SplatW() const
    {
#if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(3, 3, 3, 3));
#else
        return VectorRegisterF
        (
            m_f32[3],
            m_f32[3],
            m_f32[3],
            m_f32[3]
        );
#endif
    }

    VectorRegisterF VectorRegisterF::Zero()
    {
#if defined(NES_USE_SSE)
        return _mm_setzero_ps();
#else
        return VectorRegisterF
        (
            0, 0, 0, 0
        );
#endif
    }

    VectorRegisterF VectorRegisterF::Unit()
    {
        return Replicate(1.f);
    }

    VectorRegisterF VectorRegisterF::Nan()
    {
        return Replicate(std::numeric_limits<float>::quiet_NaN());
    }

    VectorRegisterF VectorRegisterF::Replicate(const float value)
    {
#if defined(NES_USE_SSE)
        return _mm_set1_ps(value);
#else
        return VectorRegisterF
        (
            value, value, value, value
        );
#endif
    }

    VectorRegisterF VectorRegisterF::Load(const float* pValues)
    {
#if defined(NES_USE_SSE)
        return _mm_loadu_ps(pValues);
#else
        return VectorRegisterF
        (
            pValues[0],
            pValues[1],
            pValues[2],
            pValues[3]
        );
#endif
    }

    void VectorRegisterF::Store(const VectorRegisterF& vec, float* pOutValues)
    {
#if defined(NES_USE_SSE)
        _mm_storeu_ps(pOutValues, vec.m_value);
#else
        pOutValues[0] = vec.m_f32[0];
        pOutValues[1] = vec.m_f32[1];
        pOutValues[2] = vec.m_f32[2];
        pOutValues[3] = vec.m_f32[3];
#endif
    }

    VectorRegisterF VectorRegisterF::Min(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_min_ps(a.m_value, b.m_value);
#else
        return VectorRegisterF
        (
            math::Min(a.m_f32[0], b.m_f32[0]),
            math::Min(a.m_f32[1], b.m_f32[1]),
            math::Min(a.m_f32[2], b.m_f32[2]),
            math::Min(a.m_f32[3], b.m_f32[3]),
        );
#endif
    }

    VectorRegisterF VectorRegisterF::Max(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_min_ps(a.m_value, b.m_value);
#else
        return VectorRegisterF
        (
            math::Max(a.m_f32[0], b.m_f32[0]),
            math::Max(a.m_f32[1], b.m_f32[1]),
            math::Max(a.m_f32[2], b.m_f32[2]),
            math::Max(a.m_f32[3], b.m_f32[3]),
        );
#endif
    }

    VectorRegisterF VectorRegisterF::Or(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_or_ps(a.m_value, b.m_value);
#else
        return VectorRegisterUint::Or(a.ReinterpretAsInt(), b.ReinterpretAsInt()).ReinterpretAsFloat();
#endif
    }

    VectorRegisterF VectorRegisterF::And(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_and_ps(a.m_value, b.m_value);
#else
        return VectorRegisterUint::And(a.ReinterpretAsInt(), b.ReinterpretAsInt()).ReinterpretAsFloat();
#endif
    }

    VectorRegisterF VectorRegisterF::Xor(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_xor_ps(a.m_value, b.m_value);
#else
        return VectorRegisterUint::Xor(a.ReinterpretAsInt(), b.ReinterpretAsInt()).ReinterpretAsFloat();
#endif
    }

    VectorRegisterUint VectorRegisterF::Equals(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmpeq_ps(a.m_value, b.m_value));
#else
        return VectorRegisterUint
        (
            a.m_f32[0] == b.m_f32[0] ? 0xffffffffu : 0,
            a.m_f32[1] == b.m_f32[1] ? 0xffffffffu : 0,
            a.m_f32[2] == b.m_f32[2] ? 0xffffffffu : 0,
            a.m_f32[3] == b.m_f32[3] ? 0xffffffffu : 0
        );
#endif
    }

    VectorRegisterUint VectorRegisterF::Greater(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmpgt_ps(a.m_value, b.m_value));
#else
        return VectorRegisterUint
        (
            a.m_f32[0] > b.m_f32[0] ? 0xffffffffu : 0,
            a.m_f32[1] > b.m_f32[1] ? 0xffffffffu : 0,
            a.m_f32[2] > b.m_f32[2] ? 0xffffffffu : 0,
            a.m_f32[3] > b.m_f32[3] ? 0xffffffffu : 0
        );
#endif
    }

    VectorRegisterUint VectorRegisterF::GreaterOrEqual(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmpge_ps(a.m_value, b.m_value));
#else
        return VectorRegisterUint
        (
            a.m_f32[0] >= b.m_f32[0] ? 0xffffffffu : 0,
            a.m_f32[1] >= b.m_f32[1] ? 0xffffffffu : 0,
            a.m_f32[2] >= b.m_f32[2] ? 0xffffffffu : 0,
            a.m_f32[3] >= b.m_f32[3] ? 0xffffffffu : 0
        );
#endif
    }

    VectorRegisterUint VectorRegisterF::Less(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmplt_ps(a.m_value, b.m_value));
#else
        return VectorRegisterUint
        (
            a.m_f32[0] < b.m_f32[0] ? 0xffffffffu : 0,
            a.m_f32[1] < b.m_f32[1] ? 0xffffffffu : 0,
            a.m_f32[2] < b.m_f32[2] ? 0xffffffffu : 0,
            a.m_f32[3] < b.m_f32[3] ? 0xffffffffu : 0
        );
#endif
    }

    VectorRegisterUint VectorRegisterF::LesserOrEqual(const VectorRegisterF& a, const VectorRegisterF& b)
    {
#if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmple_ps(a.m_value, b.m_value));
#else
        return VectorRegisterUint
        (
            a.m_f32[0] <= b.m_f32[0] ? 0xffffffffu : 0,
            a.m_f32[1] <= b.m_f32[1] ? 0xffffffffu : 0,
            a.m_f32[2] <= b.m_f32[2] ? 0xffffffffu : 0,
            a.m_f32[3] <= b.m_f32[3] ? 0xffffffffu : 0
        );
#endif
    }
    
    inline VectorRegisterF VectorRegisterF::Select(const VectorRegisterF& notSet, const VectorRegisterF& set,
        const VectorRegisterUint& control)
    {
#if defined NES_USE_SSE
        __m128 isSet = _mm_castsi128_ps(_mm_srai_epi32(control.m_value, 31));
        Type value = _mm_or_ps(_mm_and_ps(isSet, set.m_value), _mm_andnot_ps(isSet, notSet.m_value));
        return _mm_shuffle_ps(value, value, _MM_SHUFFLE(2, 2, 1, 0)); // "FixW() for Jolt's Vec3 class..."
#else
        VectorRegisterF result;
        for (int i = 0; i < 3; i++)
            result.m_f32[i] = (control.m_u32[i] & 0x80000000u) ? set.m_f32[i] : notSet.m_f32[i];
    #ifdef NES_FLOATING_POINT_EXCEPTIONS_ENABLED
        result.m_f32[3] = result.m_f32[2];
    #endif // JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
        return result;
#endif
    }
}
