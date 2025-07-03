// Vec4Reg.inl
#pragma once
#include "Nessie/Math/Trigonometry.h"
#include "Nessie/Math/Vec3.h"
#include "Nessie/Math/Vec4.h"
#include "UVec4Reg.h"

namespace nes
{
    Vec4Reg::Vec4Reg(const Vec3 vec)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_set_ps(vec.z, vec.z, vec.y, vec.x);
    #else
        m_f32[0] = vec.x;
        m_f32[1] = vec.y;
        m_f32[2] = vec.z;
        m_f32[3] = vec.z;
    #endif
    }

    Vec4Reg::Vec4Reg(const Vec3 vec, const float w)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_set_ps(w, vec.z, vec.y, vec.x);
    #else
        for (int i = 0; i < 3; i++)
        {
            m_f32[i] = vec[i];
        }
        m_f32[3] = w;
    #endif
    }

    Vec4Reg::Vec4Reg(const Vec4 vec)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_set_ps(vec.w, vec.z, vec.y, vec.x);
    #else
        m_f32[0] = vec.x;
        m_f32[1] = vec.y;
        m_f32[2] = vec.z;
        m_f32[3] = vec.w;
    #endif
    }
    
    Vec4Reg::Vec4Reg(const float uniformValue)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_set_ps(uniformValue, uniformValue, uniformValue, uniformValue);
    #else
        m_f32[0] = uniformValue;
        m_f32[1] = uniformValue;
        m_f32[2] = uniformValue;
        m_f32[3] = uniformValue;
    #endif
    }
    
    Vec4Reg::Vec4Reg(const float x, const float y, const float z)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_set_ps(z, z, y, x);
    #else
        m_f32[0] = x;
        m_f32[1] = y;
        m_f32[2] = z;
        m_f32[3] = z;
    #endif
    }
    
    Vec4Reg::Vec4Reg(const float x, const float y, const float z, const float w)
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

    Vec4Reg::Vec4Reg(const Float4& value)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_load_ps(&value.x);
    #else
        m_f32[0] = value.x;
        m_f32[1] = value.y;
        m_f32[2] = value.z;
        m_f32[3] = value.w;
    #endif
    }
    
    Vec4Reg Vec4Reg::operator*(const Vec4Reg& other) const
    {
    #if defined(NES_USE_SSE)
        return _mm_mul_ps(m_value, other.m_value);
    #else
        return Vec4Reg
        (
            m_f32[0] * other.m_f32[0],
            m_f32[1] * other.m_f32[1],
            m_f32[2] * other.m_f32[2],
            m_f32[3] * other.m_f32[3]
        );
    #endif
    }

    Vec4Reg& Vec4Reg::operator*=(const Vec4Reg& other)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_mul_ps(m_value, other.m_value);
    #else
        for (int i = 0; i < 4; i++)
        {
            m_f32[i] *= other.m_f32[i];
        }
    #endif
        return *this;
    }

    Vec4Reg Vec4Reg::operator*(const float value) const
    {
    #if defined(NES_USE_SSE)
        return _mm_mul_ps(m_value, _mm_set1_ps(value));
    #else
        return Vec4Reg
        (
            m_f32[0] * value,
            m_f32[1] * value,
            m_f32[2] * value,
            m_f32[3] * value
        );
    #endif
    }

    Vec4Reg& Vec4Reg::operator*=(const float value)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_mul_ps(m_value, _mm_set1_ps(value));
    #else
        for (int i = 0; i < 4; i++)
        {
            m_f32[i] *= value;
        }
    #endif
        return *this;
    }

    inline Vec4Reg operator*(const float value, const Vec4Reg& vec)
    {
    #if defined(NES_USE_SSE)
        return _mm_mul_ps(vec.m_value, _mm_set1_ps(value));
    #else
        return Vec4Reg
        (
            vec.m_f32[0] * value,
            vec.m_f32[1] * value,
            vec.m_f32[2] * value,
            vec.m_f32[3] * value
        );
    #endif
    }

    Vec4Reg Vec4Reg::operator/(const Vec4Reg& other) const
    {
    #if defined(NES_USE_SSE)
        return _mm_div_ps(m_value, other.m_value);
    #else
        return Vec4Reg
        (
            m_f32[0] / other.m_f32[0],
            m_f32[1] / other.m_f32[1],
            m_f32[2] / other.m_f32[2],
            m_f32[3] / other.m_f32[3]
        );
    #endif
    }

    Vec4Reg& Vec4Reg::operator/=(const Vec4Reg& other)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_div_ps(m_value, other.m_value);
    #else
        for (int i = 0; i < 4; i++)
        {
            m_f32[i] /= other.m_f32[i];
        }
    #endif
        return *this;
    }

    Vec4Reg Vec4Reg::operator/(const float value) const
    {
    #if defined(NES_USE_SSE)
        return _mm_div_ps(m_value, _mm_set1_ps(value));
    #else
        return Vec4Reg
        (
            m_f32[0] / value,
            m_f32[1] / value,
            m_f32[2] / value,
            m_f32[3] / value
        );
    #endif
    }

    Vec4Reg& Vec4Reg::operator/=(const float value)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_div_ps(m_value, _mm_set1_ps(value));
    #else
        for (int i = 0; i < 4; i++)
        {
            m_f32[i] /= value;
        }
    #endif
        return *this;
    }

    Vec4Reg Vec4Reg::operator+(const Vec4Reg& other) const
    {
    #if defined(NES_USE_SSE)
        return _mm_add_ps(m_value, other.m_value);
    #else
        return Vec4Reg
        (
            m_f32[0] + other.m_f32[0],
            m_f32[1] + other.m_f32[1],
            m_f32[2] + other.m_f32[2],
            m_f32[3] + other.m_f32[3]
        );
    #endif
    }

    Vec4Reg& Vec4Reg::operator+=(const Vec4Reg& other)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_add_ps(m_value, other.m_value);
    #else
        for (int i = 0; i < 4; i++)
        {
            m_f32[i] += other.m_f32[i];
        }
    #endif
        return *this;
    }

    Vec4Reg Vec4Reg::operator-(const Vec4Reg& other) const
    {
    #if defined(NES_USE_SSE)
        return _mm_sub_ps(m_value, other.m_value);
    #else
        return Vec4Reg
        (
            m_f32[0] - other.m_f32[0],
            m_f32[1] - other.m_f32[1],
            m_f32[2] - other.m_f32[2],
            m_f32[3] - other.m_f32[3]
        );
    #endif
    }

    Vec4Reg& Vec4Reg::operator-=(const Vec4Reg& other)
    {
    #if defined(NES_USE_SSE)
        m_value = _mm_sub_ps(m_value, other.m_value);
    #else
        for (int i = 0; i < 4; i++)
        {
            m_f32[i] -= other.m_f32[i];
        }
    #endif
        return *this;
    }

    Vec4Reg Vec4Reg::operator-() const
    {
    #if defined(NES_USE_SSE)
        return _mm_sub_ps(_mm_setzero_ps(), m_value);
    #else
        #ifdef NES_CROSS_PLATFORM_DETERMINISTIC
            return Vec4Reg
            (
                0.f - m_f32[0],
                0.f - m_f32[1],
                0.f - m_f32[2],
                0.f - m_f32[3]
            );
        #else
            return Vec4Reg
            (
                -m_f32[0],
                -m_f32[1],
                -m_f32[2],
                -m_f32[3]
            );
        #endif
    #endif
    }

    float Vec4Reg::GetX() const
    {
    #if defined(NES_USE_SSE)
        return _mm_cvtss_f32(m_value);
    #else
        return m_f32[0];    
    #endif
    }

    float Vec4Reg::GetY() const
    {
        return m_f32[1];
    }

    float Vec4Reg::GetZ() const
    {
        return m_f32[2];
    }

    float Vec4Reg::GetW() const
    {
        return m_f32[3];
    }
    
    Vec3 Vec4Reg::ToVec3() const
    {
        return Vec3(m_f32[0], m_f32[1], m_f32[2]);
    }

    Vec4 Vec4Reg::ToVec4() const
    {
        return Vec4(m_f32[0], m_f32[1], m_f32[2], m_f32[3]);
    }

    Vec4Reg Vec4Reg::Zero()
    {
    #if defined(NES_USE_SSE)
        return _mm_setzero_ps();
    #else
        return Vec4Reg(0.f, 0.f, 0.f, 0.f);
    #endif
    }

    Vec4Reg Vec4Reg::Replicate(const float value)
    {
    #if defined(NES_USE_SSE)
        return _mm_set1_ps(value);
    #else
        return Vec4Reg(value, value, value, value);
    #endif
    }

    Vec4Reg Vec4Reg::LoadVec4(const Vec4* pVec)
    {
    #if defined(NES_USE_SSE)
            return _mm_load_ps(&pVec->x);
    #else
            return Vec4Reg(pVec->x, pVec->y, pVec->z, pVec->w);
    #endif
    }

    Vec4Reg Vec4Reg::LoadFloat4(const Float4* pFloats)
    {
    #if defined(NES_USE_SSE)
        return _mm_loadu_ps(&pFloats->x);
    #else
        return Vec4Reg(pFloats->x, pFloats->y, pFloats->z, pFloats->w);
    #endif
    }

    Vec4Reg Vec4Reg::LoadFloat4Aligned(const Float4* pFloats)
    {
    #if defined(NES_USE_SSE)
        return _mm_load_ps(&pFloats->x);
    #else
        return Vec4Reg(pFloats->x, pFloats->y, pFloats->z, pFloats->w);
    #endif
    }

    Vec4Reg Vec4Reg::LoadFloat3Unsafe(const Float3& value)
    {
    #if defined(NES_USE_SSE)
        Type result = _mm_loadu_ps(&value.x);
    #else
        Type result = { value.x, value.y, value.z };
    #endif
        return FixW(result);
    }

    Vec4Reg Vec4Reg::LoadVec3Unsafe(const Vec3& value)
    {
    #if defined(NES_USE_SSE)
        Type result = _mm_loadu_ps(&value.x);
    #else
        Type result = { value.x, value.y, value.z };
    #endif
        return FixW(result);
    }
    
    template <const int Scale>
    Vec4Reg Vec4Reg::GatherFloat4(const float* pBase, const UVec4Reg& offsets)
    {
    #if defined(NES_USE_AVX2)
        return _mm_i32gather_ps(pBase, offsets.m_value, Scale);
    #elif defined(NES_USE_SSE)
        const uint8 *pBaseU8 = reinterpret_cast<const uint8 *>(pBase);
        Type x = _mm_load_ss(reinterpret_cast<const float*>(pBaseU8 + offsets.GetX() * Scale));
        Type y = _mm_load_ss(reinterpret_cast<const float*>(pBaseU8 + offsets.GetY() * Scale));
        const Type xy = _mm_unpacklo_ps(x, y);
        Type z = _mm_load_ss(reinterpret_cast<const float*>(pBaseU8 + offsets.GetZ() * Scale));
        Type w = _mm_load_ss(reinterpret_cast<const float*>(pBaseU8 + offsets.GetW() * Scale));
        const Type zw = _mm_unpacklo_ps(z, w);
        return _mm_movelh_ps(xy, zw);
    #else
        const uint8* pBaseU8 = reinterpret_cast<const uint8*>(pBase);
        const float x = *reinterpret_cast<const float*>(pBaseU8 + offsets.GetX() * Scale);
        const float y = *reinterpret_cast<const float*>(pBaseU8 + offsets.GetY() * Scale);
        const float z = *reinterpret_cast<const float*>(pBaseU8 + offsets.GetZ() * Scale);
        const float w = *reinterpret_cast<const float*>(pBaseU8 + offsets.GetW() * Scale);
        return Vec4Reg(x, y, z, w);
    #endif
    }
    
    Vec4Reg Vec4Reg::Min(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_min_ps(left.m_value, right.m_value);
    #else
        return Vec4Reg
        (
            math::Min(left.m_f32[0], right.m_f32[0]),
            math::Min(left.m_f32[1], right.m_f32[1]),
            math::Min(left.m_f32[2], right.m_f32[2]),
            math::Min(left.m_f32[3], right.m_f32[3])
        );
    #endif
    }

    Vec4Reg Vec4Reg::Max(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_max_ps(left.m_value, right.m_value);
    #else
        return Vec4Reg
        (
            math::Max(left.m_f32[0], right.m_f32[0]),
            math::Max(left.m_f32[1], right.m_f32[1]),
            math::Max(left.m_f32[2], right.m_f32[2]),
            math::Max(left.m_f32[3], right.m_f32[3])
        );
    #endif
    }

    UVec4Reg Vec4Reg::Equals(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmpeq_ps(left.m_value, right.m_value));
    #else
        return UVec4Reg
        (
            left.m_f32[0] == right.m_f32[0]? 0xffffffffu : 0,
            left.m_f32[1] == right.m_f32[1]? 0xffffffffu : 0,
            left.m_f32[2] == right.m_f32[2]? 0xffffffffu : 0,
            left.m_f32[3] == right.m_f32[3]? 0xffffffffu : 0
        );
    #endif
    }

    UVec4Reg Vec4Reg::Less(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmplt_ps(left.m_value, right.m_value));
    #else
        return UVec4Reg
        (
            left.m_f32[0] < right.m_f32[0]? 0xffffffffu : 0,
            left.m_f32[1] < right.m_f32[1]? 0xffffffffu : 0,
            left.m_f32[2] < right.m_f32[2]? 0xffffffffu : 0,
            left.m_f32[3] < right.m_f32[3]? 0xffffffffu : 0
        );
    #endif
    }

    UVec4Reg Vec4Reg::LessOrEqual(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmple_ps(left.m_value, right.m_value));
    #else
        return UVec4Reg
        (
            left.m_f32[0] <= right.m_f32[0]? 0xffffffffu : 0,
            left.m_f32[1] <= right.m_f32[1]? 0xffffffffu : 0,
            left.m_f32[2] <= right.m_f32[2]? 0xffffffffu : 0,
            left.m_f32[3] <= right.m_f32[3]? 0xffffffffu : 0
        );
    #endif
    }

    UVec4Reg Vec4Reg::Greater(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmpgt_ps(left.m_value, right.m_value));
    #else
        return UVec4Reg
        (
            left.m_f32[0] > right.m_f32[0]? 0xffffffffu : 0,
            left.m_f32[1] > right.m_f32[1]? 0xffffffffu : 0,
            left.m_f32[2] > right.m_f32[2]? 0xffffffffu : 0,
            left.m_f32[3] > right.m_f32[3]? 0xffffffffu : 0
        );
    #endif
    }
    
    UVec4Reg Vec4Reg::GreaterOrEqual(const Vec4Reg& left, const Vec4Reg& right)
    {
    #if defined(NES_USE_SSE)
        return _mm_castps_si128(_mm_cmpge_ps(left.m_value, right.m_value));
    #else
        return UVec4Reg
        (
            left.m_f32[0] >= right.m_f32[0]? 0xffffffffu : 0,
            left.m_f32[1] >= right.m_f32[1]? 0xffffffffu : 0,
            left.m_f32[2] >= right.m_f32[2]? 0xffffffffu : 0,
            left.m_f32[3] >= right.m_f32[3]? 0xffffffffu : 0
        );
    #endif
    }

    Vec4Reg Vec4Reg::FusedMultiplyAdd(const Vec4Reg& mul1, const Vec4Reg& mul2, const Vec4Reg& add)
    {
    #if defined(NES_USE_SSE)
        #ifdef NES_USE_FMADD
            return _mm_fmadd_ps(mul1.m_value, mul2.m_value, add.m_value);
        #else
            return _mm_add_ps(_mm_mul_ps(mul1.m_value, mul2.m_value), add.m_value);
        #endif
    #else
        return Vec4Reg
        (
            mul1.m_f32[0] * mul2.m_f32[0] + add.m_f32[0],
            mul1.m_f32[1] * mul2.m_f32[1] + add.m_f32[1],
            mul1.m_f32[2] * mul2.m_f32[2] + add.m_f32[2],
            mul1.m_f32[3] * mul2.m_f32[3] + add.m_f32[3]
        );
    #endif
    }

    Vec4Reg Vec4Reg::Select(const Vec4Reg notSet, const Vec4Reg set, const UVec4Reg mask)
    {
    #if defined(NES_USE_SSE4_1)
        return _mm_blendv_ps(notSet.m_value, set.m_value, _mm_castsi128_ps(mask.m_value));
    #elif defined (NES_USE_SSE)
        const __m128 isSet = _mm_castsi128_ps(_mm_srai_epi32(mask.m_value, 31));
        return _mm_or_ps(_mm_and_ps(isSet, set.m_value), _mm_andnot_ps(isSet, notSet.m_value));
    #else
        Vec4Reg result;
        for (int i = 0; i < 4; i++)
        {
            result.m_f32[i] = (mask.m_u32[i] & 0x80000000u) ? set.m_f32[i] : notSet.m_f32[i];
        }
        return result;
    #endif
    }

    Vec4Reg Vec4Reg::Or(const Vec4Reg left, const Vec4Reg right)
    {
    #if defined(NES_USE_SSE)
        return _mm_or_ps(left.m_value, right.m_value);
    #else
        return Vec4Reg(UVec4Reg::Or(left.ReinterpretAsInt(), right.ReinterpretAsInt()).ReinterpretAsFloat());
    #endif
    }

    Vec4Reg Vec4Reg::Xor(const Vec4Reg left, const Vec4Reg right)
    {
    #if defined(NES_USE_SSE)
        return _mm_xor_ps(left.m_value, right.m_value);
    #else
        return Vec4Reg(UVec4Reg::Xor(left.ReinterpretAsInt(), right.ReinterpretAsInt()).ReinterpretAsFloat());
    #endif
    }

    Vec4Reg Vec4Reg::And(const Vec4Reg left, const Vec4Reg right)
    {
    #if defined(NES_USE_SSE)
        return _mm_and_ps(left.m_value, right.m_value);
    #else
        return Vec4Reg(UVec4Reg::And(left.ReinterpretAsInt(), right.ReinterpretAsInt()).ReinterpretAsFloat());
    #endif
    }

    bool Vec4Reg::IsNaN() const
    {
    #if defined(NES_USE_AVX512)
        return _mm_fpclass_ps_mask(m_value, 0b10000001) != 0;
    #elif defined(NES_USE_SSE)
        return _mm_movemask_ps(_mm_cmpunord_ps(m_value, m_value)) != 0;
    #else
        return math::IsNan(m_f32[0]) || math::IsNan(m_f32[1]) || math::IsNan(m_f32[2]) || math::IsNan(m_f32[3]);
    #endif
        
    }

    Vec4Reg Vec4Reg::SplatX() const
    {
    #if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(0, 0, 0, 0));
    #else
        return Vec4Reg(m_f32[0], m_f32[0], m_f32[0], m_f32[0]);
    #endif
    }

    Vec4Reg Vec4Reg::SplatY() const
    {
    #if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(1, 1, 1, 1));
    #else
        return Vec4Reg(m_f32[1], m_f32[1], m_f32[1], m_f32[1]);
    #endif
    }

    Vec4Reg Vec4Reg::SplatZ() const
    {
    #if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(2, 2, 2, 2));
    #else
        return Vec4Reg(m_f32[2], m_f32[2], m_f32[2], m_f32[2]);
    #endif
    }

    Vec4Reg Vec4Reg::SplatW() const
    {
    #if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(3, 3, 3, 3));
    #else
        return Vec4Reg(m_f32[3], m_f32[3], m_f32[3], m_f32[3]);
    #endif
    }

    template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
    Vec4Reg Vec4Reg::Swizzle() const
    {
        static_assert(SwizzleX <= 3, "SwizzleX out of range!");
        static_assert(SwizzleY <= 3, "SwizzleY out of range!");
        static_assert(SwizzleZ <= 3, "SwizzleZ out of range!");
        static_assert(SwizzleW <= 3, "SwizzleW out of range!");

    #if defined(NES_USE_SSE)
        return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(SwizzleW, SwizzleZ, SwizzleY, SwizzleX));
    #else
        return Vec4Reg(m_f32[SwizzleX], m_f32[SwizzleY], m_f32[SwizzleZ], m_f32[SwizzleW]);
    #endif
    }

    Vec4Reg Vec4Reg::Abs() const
    {
    #if defined (NES_USE_AVX512)
        return _mm_range_ps(m_value, m_value, 0b1000);
    #elif defined(NES_USE_SSE)
        return _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), m_value), m_value);
    #else
        return Vec4Reg(math::Abs(m_f32[0]), math::Abs(m_f32[1]), math::Abs(m_f32[2]), math::Abs(m_f32[3]));
    #endif
    }

    Vec4Reg Vec4Reg::DotV(const Vec4Reg& other) const
    {
    #if defined(NES_USE_SSE4_1)
        return _mm_dp_ps(m_value, other.m_value, 0xff);
    #else
        float dot = 0.f;
        for (int i = 0; i < 4; i++)
        {
            dot += m_f32[i] * other.m_f32[i];
        }
        return Vec4Reg::Replicate(dot);
    #endif
    }

    float Vec4Reg::Dot(const Vec4Reg& other) const
    {
    #if defined(NES_USE_SSE4_1)
        return _mm_cvtss_f32(_mm_dp_ps(m_value, other.m_value, 0xff));
    #else
        float dot = 0.f;
        for (int i = 0; i < 4; i++)
        {
            dot += m_f32[i] * other.m_f32[i];
        }
        return dot;
    #endif
    }

    float Vec4Reg::Length3(const Vec3 vec)
    {
        const Vec4Reg result(vec.x, vec.y, vec.z, 0.f);
        return result.Length();
    }

    float Vec4Reg::LengthSqr3(const Vec3 vec)
    {
        const Vec4Reg result(vec.x, vec.y, vec.z, 0.f);
        return result.LengthSqr();
    }

    Vec3 Vec4Reg::NormalizedOr3(const Vec3 vec, const Vec3 zeroValue)
    {
    #if defined(NES_USE_SSE4_1)
        const Vec4Reg vecReg(vec.x, vec.y, vec.z, 0.f);
        const Vec4Reg zeroValueReg(zeroValue.x, zeroValue.y, zeroValue.z, 0.f);
        Type lengthSqr = _mm_dp_ps(vecReg.m_value, vecReg.m_value, 0x7f);
        // clang with '-ffast-math' (which you should not use!) can generate _mm_rsqrt_ps
        // instructions which produce INFs/NaNs when they get a denormal float as input.
        // We therefore treat denormals as zero here.
        Type isZero = _mm_cmple_ps(lengthSqr, _mm_set1_ps(FLT_MIN));
        #ifdef NES_FLOATING_POINT_EXCEPTIONS_ENABLED
            if (_mm_movemask_ps(isZero) == 0xf)
                return zeroValue;
            const Vec4Reg result = _mm_div_ps(vecReg.m_value, _mm_sqrt_ps(lengthSqr));
            return result.ToVec3();
        #else
            const Vec4Reg result = _mm_blendv_ps(_mm_div_ps(vecReg.m_value, _mm_sqrt_ps(lengthSqr)), zeroValueReg.m_value, isZero);
            return result.ToVec3();
        #endif    

    #else
        const float lengthSqr = vec.LengthSqr();
        if (lengthSqr <= FLT_MIN)
            return zeroValue;
        return vec / std::sqrt(lengthSqr);
    #endif
    }

    Vec3 Vec4Reg::Cross3(const Vec3 a, const Vec3 b)
    {
    #if defined(NES_USE_SSE)
        const Vec4Reg aReg(a);
        const Vec4Reg bReg(b);
        
        Type t1 = _mm_shuffle_ps(bReg.m_value, bReg.m_value, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same.
        t1 = _mm_mul_ps(t1, aReg.m_value);
        Type t2 = _mm_shuffle_ps(aReg.m_value, aReg.m_value, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same.
        t2 = _mm_mul_ps(t2, bReg.m_value);
        const Type t3 = _mm_sub_ps(t1, t2);
        
        const Vec4Reg result = _mm_shuffle_ps(t3, t3, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same.
        return result.ToVec3();
    #else
        return Vec3
        (
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    #endif           
    }

    float Vec4Reg::LengthSqr() const
    {
    #if defined(NES_USE_SSE4_1)
        return _mm_cvtss_f32(_mm_dp_ps(m_value, m_value, 0xff));
    #else
        float lengthSqr = 0.f;
        for (int i = 0; i < 4; i++)
        {
            lengthSqr += m_f32[i] * m_f32[i];
        }
        return lengthSqr;
    #endif
    }

    float Vec4Reg::Length() const
    {
    #if defined(NES_USE_SSE4_1)
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(m_value, m_value, 0xff)));
    #else
        return std::sqrt(LengthSqr());
    #endif
    }

    Vec4Reg Vec4Reg::Normalized() const
    {
    #if defined(NES_USE_SSE)
        return _mm_div_ps(m_value, _mm_sqrt_ps(_mm_dp_ps(m_value, m_value, 0xff)));
    #else
        return *this / Length();
    #endif
    }

    void Vec4Reg::StoreFloat4(Float4* pOutFloats) const
    {
    #if defined(NES_USE_SSE)
        _mm_store_ps(&pOutFloats->x, m_value);
    #else
        pOutFloats->x = m_f32[0];
        pOutFloats->y = m_f32[1];
        pOutFloats->z = m_f32[2];
        pOutFloats->w = m_f32[3];
    #endif
    }

    void Vec4Reg::StoreVec4(Vec4* pOutVec) const
    {
    #if defined(NES_USE_SSE)
        _mm_store_ps(&pOutVec->x, m_value);
    #else
        pOutVec->x = m_f32[0];
        pOutVec->y = m_f32[1];
        pOutVec->z = m_f32[2];
        pOutVec->w = m_f32[3];
    #endif
    }

    UVec4Reg Vec4Reg::ToInt() const
    {
    #if defined(NES_USE_SSE)
        return _mm_cvttps_epi32(m_value);
    #else
        return UVec4Reg
        (
            static_cast<uint32>(m_f32[0]),
            static_cast<uint32>(m_f32[1]),
            static_cast<uint32>(m_f32[2]),
            static_cast<uint32>(m_f32[3])
        );
    #endif
    }

    UVec4Reg Vec4Reg::ReinterpretAsInt() const
    {
    #if defined(NES_USE_SSE)
        return UVec4Reg(_mm_castps_si128(m_value));
    #else
        return *reinterpret_cast<const UVec4Reg*>(this);
    #endif
    }

    int Vec4Reg::GetSignBits() const
    {
    #if defined(NES_USE_SSE)
        return _mm_movemask_ps(m_value);
    #else
        return (std::signbit(m_f32[0])? 1 : 0)
            | (std::signbit(m_f32[1])? 2 : 0)
            | (std::signbit(m_f32[2])? 4 : 0)
            | (std::signbit(m_f32[3])? 8 : 0);
    #endif
    }
    
    Vec4Reg Vec4Reg::Sqrt() const
    {
    #if defined(NES_USE_SSE)
        return _mm_sqrt_ps(m_value);
    #else
        return Vec4Reg(std::sqrt(m_f32[0]), std::sqrt(m_f32[1]), std::sqrt(m_f32[2]), std::sqrt(m_f32[3]));
    #endif
    }

    Vec4Reg Vec4Reg::GetSign() const
    {
    #if defined (NES_USE_AVX512)
        return _mm_fixupimm_ps(m_value, m_value, _mm_set1_epi32(0xA9A90A00), 0);    
    #elif defined(NES_USE_SSE)
        const Type minusOne = _mm_set1_ps(-1.0f);
        const Type one = _mm_set1_ps(1.0f);
        return _mm_or_ps(_mm_and_ps(m_value, minusOne), one);
    #else
        return Vec4Reg
        (
            std::signbit(m_f32[0])? -1.0f : 1.0f,
            std::signbit(m_f32[1])? -1.0f : 1.0f,
            std::signbit(m_f32[2])? -1.0f : 1.0f,
            std::signbit(m_f32[3])? -1.0f : 1.0f
        );
    #endif
    }

    void Vec4Reg::CheckW() const
    {
    #ifdef NES_FLOATING_POINT_EXCEPTIONS_ENABLED
        NES_ASSERT(reinterpret_cast<const uint32*>(&m_f32)[2] == reinterpret_cast<const uint32*>(&m_f32)[3]);
    #endif
    }

    Vec4Reg::Type Vec4Reg::FixW(Type value)
    {
    #ifdef NES_FLOATING_POINT_EXCEPTIONS_ENABLED
        #if defined(NES_USE_SSE)
            return _mm_shuffle_ps(value, value, _MM_SHUFFLE(2, 2, 1, 0));
        #else
            Type result;
            result.m_f32[0] = value.m_f32[0];
            result.m_f32[1] = value.m_f32[1];
            result.m_f32[2] = value.m_f32[2];
            result.m_f32[3] = value.m_f32[2];
            return result;
        #endif
    #else
        return value;
    #endif   
    }

    bool Vec4Reg::operator==(const Vec4Reg& other) const
    {
        return Vec4Reg::Equals(*this, other).TestAllTrue(); 
    }
    
    Vec4Reg& Vec4Reg::Normalize()
    {
        *this = *this / Length();
        return *this;
    }
    
    bool Vec4Reg::IsClose(const Vec4Reg& other, const float maxDistSqr) const
    {
        return (other - *this).LengthSqr() <= maxDistSqr;
    }

    bool Vec4Reg::IsNormalized(const float tolerance) const
    {
        return math::Abs(LengthSqr() - 1.0f) <= tolerance;
    }

    float Vec4Reg::MinComponent() const
    {
        Vec4Reg result = Min(m_value, Swizzle<ESwizzleY, ESwizzleUnused, ESwizzleW, ESwizzleUnused>());
        result = Min(result, result.Swizzle<ESwizzleZ, ESwizzleUnused, ESwizzleUnused, ESwizzleUnused>());
        return result.m_f32[0];
    }

    float Vec4Reg::MaxComponent() const
    {
        Vec4Reg result = Max(m_value, Swizzle<ESwizzleY, ESwizzleUnused, ESwizzleW, ESwizzleUnused>());
        result = Max(result, result.Swizzle<ESwizzleZ, ESwizzleUnused, ESwizzleUnused, ESwizzleUnused>());
        return result.m_f32[0];
    }
    
    void Vec4Reg::Sort4(Vec4Reg& value, UVec4Reg& index)
    {
        // Pass 1, test 1st vs 3rd, 2nd vs 4th
        Vec4Reg v1 = value.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleX, ESwizzleY>();
        UVec4Reg i1 = index.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleX, ESwizzleY>();
        UVec4Reg c1 = Less(value, v1).Swizzle<ESwizzleZ, ESwizzleW, ESwizzleZ, ESwizzleW>();
        value = Select(value, v1, c1);
        index = UVec4Reg::Select(index, i1, c1);

        // Pass 2, test 1st vs 2nd, 3rd vs 4th
        Vec4Reg v2 = value.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>();
        UVec4Reg i2 = index.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>();
        UVec4Reg c2 = Less(value, v2).Swizzle<ESwizzleY, ESwizzleY, ESwizzleW, ESwizzleW>();
        value = Select(value, v2, c2);
        index = UVec4Reg::Select(index, i2, c2);

        // Pass 3, test 2nd vs 3rd component
        Vec4Reg v3 = value.Swizzle<ESwizzleX, ESwizzleZ, ESwizzleY, ESwizzleW>();
        UVec4Reg i3 = index.Swizzle<ESwizzleX, ESwizzleZ, ESwizzleY, ESwizzleW>();
        UVec4Reg c3 = Less(value, v3).Swizzle<ESwizzleX, ESwizzleZ, ESwizzleZ, ESwizzleW>();
        value = Select(value, v3, c3);
        index = UVec4Reg::Select(index, i3, c3);
    }

    void Vec4Reg::Sort4Reverse(Vec4Reg& value, UVec4Reg& index)
    {
        // Pass 1, test 1st vs 3rd, 2nd vs 4th
        Vec4Reg v1 = value.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleX, ESwizzleY>();
        UVec4Reg i1 = index.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleX, ESwizzleY>();
        UVec4Reg c1 = Greater(value, v1).Swizzle<ESwizzleZ, ESwizzleW, ESwizzleZ, ESwizzleW>();
        value = Select(value, v1, c1);
        index = UVec4Reg::Select(index, i1, c1);

        // Pass 2, test 1st vs 2nd, 3rd vs 4th
        Vec4Reg v2 = value.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>();
        UVec4Reg i2 = index.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>();
        UVec4Reg c2 = Greater(value, v2).Swizzle<ESwizzleY, ESwizzleY, ESwizzleW, ESwizzleW>();
        value = Select(value, v2, c2);
        index = UVec4Reg::Select(index, i2, c2);

        // Pass 3, test 2nd vs 3rd component
        Vec4Reg v3 = value.Swizzle<ESwizzleX, ESwizzleZ, ESwizzleY, ESwizzleW>();
        UVec4Reg i3 = index.Swizzle<ESwizzleX, ESwizzleZ, ESwizzleY, ESwizzleW>();
        UVec4Reg c3 = Greater(value, v3).Swizzle<ESwizzleX, ESwizzleZ, ESwizzleZ, ESwizzleW>();
        value = Select(value, v3, c3);
        index = UVec4Reg::Select(index, i3, c3);
    }
    
    void Vec4Reg::SinCos(Vec4Reg& outSin, Vec4Reg& outCos) const
    {
        // Implementation based on sinf.c from the cephes library, combines sinf and cosf in a single function, changes octants to quadrants and vectorizes it
        // Original implementation by Stephen L. Moshier (See: http://www.moshier.net/)

        // Make argument positive and remember sign for sin only since cos is symmetric around x (the highest bit of a float is the sign bit).
        UVec4Reg sinSign = UVec4Reg::And(ReinterpretAsInt(), UVec4Reg::Replicate(0x80000000U));
        Vec4Reg xVec = Vec4Reg::Xor(*this, sinSign.ReinterpretAsFloat());

        // x / (PI / 2) rounded to the nearest int gives us the quadrant closest to x
        UVec4Reg quadrant = (0.6366197723675814f * xVec + Vec4Reg::Replicate(0.5f)).ToInt();

        // Make x relative to the closest quadrant.
        // This does x = x - quadrant * PI / 2 using a two-step Cody-Waite argument reduction.
        // This improves the accuracy of the result by avoiding loss of significant bits in the subtraction.
        // We start with x = x - quadrant * PI / 2, PI / 2 in hexadecimal notation is 0x3fc90fdb, we remove the lowest 16 bits to
        // get 0x3fc90000 (= 1.5703125) this means we can now multiply with a number of up to 2^16 without losing any bits.
        // This leaves us with: x = (x - quadrant * 1.5703125) - quadrant * (PI / 2 - 1.5703125).
        // PI / 2 - 1.5703125 in hexadecimal is 0x39fdaa22, stripping the lowest 12 bits we get 0x39fda000 (= 0.0004837512969970703125)
        // This leaves uw with: x = ((x - quadrant * 1.5703125) - quadrant * 0.0004837512969970703125) - quadrant * (PI / 2 - 1.5703125 - 0.0004837512969970703125)
        // See: https://stackoverflow.com/questions/42455143/sine-cosine-modular-extended-precision-arithmetic
        // After this we have x in the range [-PI / 4, PI / 4].
        Vec4Reg floatQuadrant = quadrant.ToFloat();
        xVec = ((xVec - floatQuadrant * 1.5703125f) - floatQuadrant *  0.0004837512969970703125f) - floatQuadrant * 7.549789948768648e-8f;

        // Calculate x2 = x^2
        Vec4Reg x2 = xVec * xVec;

        // Taylor expansion:
        // Cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + x^8/8! + ... = (((x2/8!- 1/6!) * x2 + 1/4!) * x2 - 1/2!) * x2 + 1
        Vec4Reg taylorCos = ((2.443315711809948e-5f * x2 - Vec4Reg::Replicate(1.388731625493765e-3f)) * x2 + Vec4Reg::Replicate(4.166664568298827e-2f)) * x2 * x2 - 0.5f * x2 + Vec4Reg::One();
        // Sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ... = ((-x2/7! + 1/5!) * x2 - 1/3!) * x2 * x + x
        Vec4Reg taylorSin = ((-1.9515295891e-4f * x2 + Vec4Reg::Replicate(8.3321608736e-3f)) * x2 - Vec4Reg::Replicate(1.6666654611e-1f)) * x2 * xVec + xVec;

        // The lowest 2 bits of quadrant indicate the quadrant that we are in.
        // Let x be the original input value and x' our value that has been mapped to the range [-PI / 4, PI / 4].
        // since cos(x) = sin(x - PI / 2) and since we want to use the Taylor expansion as close as possible to 0,
        // we can alternate between using the Taylor expansion for sin and cos according to the following table:
        //
        // quadrant	 sin(x)		 cos(x)
        // XXX00b	 sin(x')	 cos(x')
        // XXX01b	 cos(x')	-sin(x')
        // XXX10b	-sin(x')	-cos(x')
        // XXX11b	-cos(x')	 sin(x')
        //
        // So: sin_sign = bit2, cos_sign = bit1 ^ bit2, bit1 determines if we use sin or cos Taylor expansion
        UVec4Reg bit1 = quadrant.LogicalShiftLeft<31>();
        UVec4Reg bit2 = UVec4Reg::And(quadrant.LogicalShiftLeft<30>(), UVec4Reg::Replicate(0x80000000U));

        // Select which one of the results is sin, and which one is cos
        Vec4Reg sin = Vec4Reg::Select(taylorSin, taylorCos, bit1);
        Vec4Reg cos = Vec4Reg::Select(taylorCos, taylorSin, bit1);

        // Update the signs
        sinSign = UVec4Reg::Xor(sinSign, bit2);
        UVec4Reg cosSign = UVec4Reg::Xor(bit1, bit2);

        // Correct the signs
        outSin = Vec4Reg::Xor(sin, sinSign.ReinterpretAsFloat());
        outCos = Vec4Reg::Xor(cos, cosSign.ReinterpretAsFloat());
    }

    Vec4Reg Vec4Reg::Tan() const
    {
        // Implementation based on tanf.c from the cephes library, see Vec4Reg::SinCos for further details
        // Original implementation by Stephen L. Moshier (See: http://www.moshier.net/)

        // Make argument positive and remember the sign
        const UVec4Reg tanSign = UVec4Reg::And(Vec4Reg::ReinterpretAsInt(), UVec4Reg::Replicate(0x80000000U));
        Vec4Reg xVec = Vec4Reg::Xor(*this, tanSign.ReinterpretAsFloat());

        // x / (PI / 2) rounded to the nearest int gives us the quadrant closest to x
        const UVec4Reg quadrant = (0.6366197723675814f * xVec + Vec4Reg::Replicate(0.5f)).ToInt();

        // Remap x to range [-PI / 4, PI / 4], see Vec4Reg::SinCos
        const Vec4Reg floatQuadrant = quadrant.ToFloat();
        xVec = ((xVec - floatQuadrant * 1.5703125f) - floatQuadrant * 0.0004837512969970703125f) - floatQuadrant * 7.549789948768648e-8f;

        // Calculate x2 = x^2
        const Vec4Reg x2 = xVec * xVec;
        
        // Roughly equivalent to the Taylor expansion:
        // Tan(x) = x + x^3/3 + 2*x^5/15 + 17*x^7/315 + 62*x^9/2835 + ...
        Vec4Reg tan =
            (((((9.38540185543e-3f * x2 + Vec4Reg::Replicate(3.11992232697e-3f)) * x2 + Vec4Reg::Replicate(2.44301354525e-2f)) * x2
            + Vec4Reg::Replicate(5.34112807005e-2f)) * x2 + Vec4Reg::Replicate(1.33387994085e-1f)) * x2 + Vec4Reg::Replicate(3.33331568548e-1f)) * x2 * xVec + xVec;

        // For the 2nd and 4th quadrant we need to invert the value
        const UVec4Reg bit1 = quadrant.LogicalShiftLeft<31>();
        // Add small epsilon to prevent div by zero, works because tan is always positive
        tan = Vec4Reg::Select(tan, Vec4Reg::Replicate(-1.f) / (tan NES_IF_FLOATING_POINT_EXCEPTIONS_ENABLED(+ Vec4Reg::Replicate(FLT_MIN))), bit1);

        // Put the sign back
        return Vec4Reg::Xor(tan, tanSign.ReinterpretAsFloat());
    }

    Vec4Reg Vec4Reg::ASin() const
    {
        // Implementation based on asinf.c from the cephes library
        // Original implementation by Stephen L. Moshier (See: http://www.moshier.net/)

        // Make argument positive and remember the sign
        UVec4Reg aSinSign = UVec4Reg::And(ReinterpretAsInt(), UVec4Reg::Replicate(0x80000000U));
        Vec4Reg a = Vec4Reg::Xor(*this, aSinSign.ReinterpretAsFloat());

        // ASin is not defined outside the range [-1, 1], but it often happens that a value is slightly above 1, so we just clamp here
        a = Vec4Reg::Min(a, Vec4Reg::One());
        
        // When |x| <= 0.5 we use the asin approximation as is
        Vec4Reg z1 = a * a;
        Vec4Reg x1 = a;

        // When |x| > 0.5 we use the identity asin(x) = PI / 2 - 2 * asin(sqrt((1 - x) / 2))
        Vec4Reg z2 = 0.5f * (Vec4Reg::One() - a);
        Vec4Reg x2 = z2.Sqrt();

        // Select which of the two situations we have
        UVec4Reg greater = Vec4Reg::Greater(a, Vec4Reg::Replicate(0.5f));
        Vec4Reg zVec = Vec4Reg::Select(z1, z2, greater);
        Vec4Reg xVec = Vec4Reg::Select(x1, x2, greater);

        // Polynomial approximation of asin.
        zVec = ((((4.2163199048e-2f * zVec + Vec4Reg::Replicate(2.4181311049e-2f)) * zVec + Vec4Reg::Replicate(4.5470025998e-2f)) * zVec + Vec4Reg::Replicate(7.4953002686e-2f)) * zVec + Vec4Reg::Replicate(1.6666752422e-1f)) * zVec * xVec + xVec;

        // If |x| > 0.5, we need to apply the remainder of the identity above.
        zVec = Vec4Reg::Select(zVec, Vec4Reg::Replicate(0.5f * math::Pi<float>()) - (zVec + zVec), greater);

        // Put the sign back
        return Vec4Reg::Xor(zVec, aSinSign.ReinterpretAsFloat());
    }

    Vec4Reg Vec4Reg::ACos() const
    {
        // Not the most accurate, but simple
        return Vec4Reg::Replicate(0.5f * math::Pi<float>()) - ASin();
    }

    Vec4Reg Vec4Reg::ATan() const
    {
        // Implementation based on atanf.c from the cephes library
        // Original implementation by Stephen L. Moshier (See: http://www.moshier.net/)

        // Make argument positive and remember the sign
        UVec4Reg aTanSign = UVec4Reg::And(ReinterpretAsInt(), UVec4Reg::Replicate(0x80000000U));
        Vec4Reg xVec = Vec4Reg::Xor(*this, aTanSign.ReinterpretAsFloat());
        Vec4Reg yVec = Vec4Reg::Zero();

        // If x > Tan(PI / 8)
        UVec4Reg greater1 = Vec4Reg::Greater(xVec, Vec4Reg::Replicate(0.4142135623730950f));
        Vec4Reg x1 = (xVec - Vec4Reg::One()) / (xVec + Vec4Reg::One());

        // If x > Tan(3 * PI / 8)
        UVec4Reg greater2 = Vec4Reg::Greater(xVec, Vec4Reg::Replicate(2.414213562373095f));
        // Add small epsilon to prevent div by zero, works because x is always positive
        Vec4Reg x2 = Vec4Reg::Replicate(-1.0f) / (xVec NES_IF_FLOATING_POINT_EXCEPTIONS_ENABLED(+ Vec4Reg::Replicate(FLT_MIN)));

        // Apply first if
        xVec = Vec4Reg::Select(xVec, x1, greater1);
        yVec = Vec4Reg::Select(yVec, Vec4Reg::Replicate(0.25f * math::Pi<float>()), greater1);

        // Apply second if
        xVec = Vec4Reg::Select(xVec, x2, greater2);
        yVec = Vec4Reg::Select(yVec, Vec4Reg::Replicate(0.5f * math::Pi<float>()), greater2);

        // Polynomial approximation
        Vec4Reg zVec = xVec * xVec;
        yVec += (((8.05374449538e-2f * zVec - Vec4Reg::Replicate(1.38776856032e-1f)) * zVec + Vec4Reg::Replicate(1.99777106478e-1f)) * zVec - Vec4Reg::Replicate(3.33329491539e-1f)) * zVec * xVec + xVec;

        // Put the sign back
        return Vec4Reg::Xor(yVec, aTanSign.ReinterpretAsFloat());
    }

    Vec4Reg Vec4Reg::ATan2(const Vec4Reg& y, const Vec4Reg& x)
    {
        UVec4Reg signMask = UVec4Reg::Replicate(0x80000000U);

        // Determine absolute value and sign of y
        UVec4Reg ySign = UVec4Reg::And(y.ReinterpretAsInt(), signMask);
        Vec4Reg yAbs = Vec4Reg::Xor(y, ySign.ReinterpretAsFloat());

        // Determine absolute value and sign of x
        UVec4Reg xSign = UVec4Reg::And(x.ReinterpretAsInt(), signMask);
        Vec4Reg xAbs = Vec4Reg::Xor(x, xSign.ReinterpretAsFloat());

        // Always divide smallest / largest to avoid dividing by zero
        UVec4Reg xIsNumerator = Vec4Reg::Less(xAbs, yAbs);
        Vec4Reg numerator = Vec4Reg::Select(yAbs, xAbs, xIsNumerator);
        Vec4Reg denominator = Vec4Reg::Select(xAbs, yAbs, xIsNumerator);
        Vec4Reg atan = (numerator / denominator).ATan();

        // If we calculated x / y instead of y / x the result is PI / 2 - result (note that this is true because we know the result is positive because the input was positive)
        atan = Vec4Reg::Select(atan, Vec4Reg::Replicate(0.5f * math::Pi<float>()) - atan, xIsNumerator);

        // Now we need to map to the correct quadrant
        // xSign	ySign	result
        // +1		+1		atan
        // -1		+1		-atan + PI
        // -1		-1		atan - PI
        // +1		-1		-atan
        // This can be written as: x_sign * y_sign * (atan - (x_sign < 0? PI : 0))
        atan -= Vec4Reg::And(xSign.ArithmeticShiftRight<31>().ReinterpretAsFloat(), Vec4Reg::Replicate(math::Pi<float>()));
        atan = Vec4Reg::Xor(atan, UVec4Reg::Xor(xSign, ySign).ReinterpretAsFloat());
        return atan;
    }
}