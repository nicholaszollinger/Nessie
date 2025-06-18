// Mat4.inl
#pragma once
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Quat.h"
#include "Rotation.h"

namespace nes
{
    /// Quick Row Column accessor. Only used in this file - it is undefined at the bottom.
#define NES_RC(row, col) m_columns[col].m_f32[row]

    Mat44::Mat44(const Vec3& diagonal)
    {
        m_columns[0] = Vec4Reg(diagonal.x, 0.f, 0.f, 0.f);
        m_columns[1] = Vec4Reg(0.f, diagonal.y, 0.f, 0.f);
        m_columns[2] = Vec4Reg(0.f, 0.f, diagonal.y, 0.f);
        m_columns[3] = Vec4Reg(0.f, 0.f, 0.f, 1.f);
    }

    Mat44::Mat44(const float uniformDiagonal)
    {
        m_columns[0] = Vec4Reg(uniformDiagonal, 0.f, 0.f, 0.f);
        m_columns[1] = Vec4Reg(0.f, uniformDiagonal, 0.f, 0.f);
        m_columns[2] = Vec4Reg(0.f, 0.f, uniformDiagonal, 0.f);
        m_columns[3] = Vec4Reg(0.f, 0.f, 0.f, 1.f);
    }
    
    Mat44::Mat44(const Vec4& c1, const Vec4& c2, const Vec4& c3, const Vec4& c4)
        : m_columns{ c1, c2, c3, c4 }
    {
        //
    }

    Mat44::Mat44(const Vec4Reg& c1, const Vec4Reg& c2, const Vec4Reg& c3, const Vec4Reg& c4)
        : m_columns{ c1, c2, c3, c4 }
    {
        //
    }

    Mat44::Mat44(const Vec4& c1, const Vec4& c2, const Vec4& c3, const Vec3& c4)
        : m_columns { c1, c2, c3, Vec4Reg(c4, 1.0f) }
    {
        //
    }

    Mat44::Mat44(Type c1, Type c2, Type c3, Type c4)
        : m_columns{ c1, c2, c3, c4 }
    {
        //
    }

    Vec4Reg Mat44::operator[](const uint index) const
    {
        NES_ASSERT(index < 4);
        return m_columns[index];
    }

    Vec4Reg& Mat44::operator[](const uint index)
    {
        NES_ASSERT(index < 4);
        return m_columns[index];
    }

    bool Mat44::operator==(const Mat44& other) const
    {
        return UVec4Reg::And(
            UVec4Reg::And(Vec4Reg::Equals(m_columns[0], other.m_columns[0]), Vec4Reg::Equals(m_columns[1], other.m_columns[1])),
            UVec4Reg::And(Vec4Reg::Equals(m_columns[2], other.m_columns[2]), Vec4Reg::Equals(m_columns[3], other.m_columns[3]))
        ).TestAllTrue();
    }

    Mat44 Mat44::operator*(const Mat44& other) const
    {
        Mat44 result;
    #if defined (NES_USE_SSE)
        for (int i = 0; i < 4; ++i)
        {
            const __m128 c = other.m_columns[i].m_value;
            __m128 t = _mm_mul_ps(m_columns[0].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0)));
            t = _mm_add_ps(t, _mm_mul_ps(m_columns[1].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))));
            t = _mm_add_ps(t, _mm_mul_ps(m_columns[2].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))));
            t = _mm_add_ps(t, _mm_mul_ps(m_columns[3].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 3, 3, 3))));
            result.m_columns[i].m_value = t;
        }
    #else
        for (int i = 0; i < 4; ++i)
        {
            result.m_columns[i] = m_columns[0] * other.m_columns[i].m_f32[0] + m_columns[1] * other.m_columns[i].m_f32[1] + m_columns[2] * other.m_columns[i].m_f32[2] + m_columns[3] * other.m_columns[i].m_f32[3];
        }
    #endif
        return result;
    }

    Vec3 Mat44::operator*(const Vec3& vec) const
    {
    #if defined (NES_USE_SSE)
        const Vec4Reg rVec = Vec4Reg(vec);
        
        __m128 t = _mm_mul_ps(m_columns[0].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(0, 0, 0, 0)));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[1].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(1, 1, 1, 1))));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[2].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(2, 2, 2, 2))));
        t = _mm_add_ps(t, m_columns[3].m_value);
        t = Vec4Reg::FixW(t);
        return Vec4Reg(t).ToVec3();
    #else
        return Vec3
        (
            m_columns[0].m_f32[0] * vec.x + m_columns[1].m_f32[0] * vec.y + m_columns[2].m_f32[0] * vec.z + m_columns[3].m_f32[0],
            m_columns[0].m_f32[1] * vec.x + m_columns[1].m_f32[1] * vec.y + m_columns[2].m_f32[1] * vec.z + m_columns[3].m_f32[1],
            m_columns[0].m_f32[2] * vec.x + m_columns[1].m_f32[2] * vec.y + m_columns[2].m_f32[2] * vec.z + m_columns[3].m_f32[2]
        );
    #endif
    }

    Vec4 Mat44::operator*(const Vec4& vec) const
    {
    #if defined (NES_USE_SSE)
        const Vec4Reg rVec = Vec4Reg(vec);
        
        __m128 t = _mm_mul_ps(m_columns[0].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(0, 0, 0, 0)));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[1].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(1, 1, 1, 1))));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[2].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(2, 2, 2, 2))));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[3].m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(3, 3, 3, 3))));
        return Vec4Reg(t).ToVec4();
    #else
        return Vec4
        (
            m_columns[0].m_f32[0] * vec.x + m_columns[1].m_f32[0] * vec.y + m_columns[2].m_f32[0] * vec.z + m_columns[3].m_f32[0] * vec.w,
            m_columns[0].m_f32[1] * vec.x + m_columns[1].m_f32[1] * vec.y + m_columns[2].m_f32[1] * vec.z + m_columns[3].m_f32[1] * vec.w,
            m_columns[0].m_f32[2] * vec.x + m_columns[1].m_f32[2] * vec.y + m_columns[2].m_f32[2] * vec.z + m_columns[3].m_f32[2] * vec.w,
            m_columns[0].m_f32[3] * vec.x + m_columns[1].m_f32[3] * vec.y + m_columns[2].m_f32[3] * vec.z + m_columns[3].m_f32[3] * vec.w
        );
    #endif
    }

    Vec4Reg Mat44::operator*(const Vec4Reg& vec) const
    {
#if defined (NES_USE_SSE)
        __m128 t = _mm_mul_ps(m_columns[0].m_value, _mm_shuffle_ps(vec.m_value, vec.m_value, _MM_SHUFFLE(0, 0, 0, 0)));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[1].m_value, _mm_shuffle_ps(vec.m_value, vec.m_value, _MM_SHUFFLE(1, 1, 1, 1))));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[2].m_value, _mm_shuffle_ps(vec.m_value, vec.m_value, _MM_SHUFFLE(2, 2, 2, 2))));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[3].m_value, _mm_shuffle_ps(vec.m_value, vec.m_value, _MM_SHUFFLE(3, 3, 3, 3))));
        return t;
#else
        return Vec4
        (
            m_columns[0].m_f32[0] * vec.m_f32[0] + m_columns[1].m_f32[0] * vec.m_f32[1] + m_columns[2].m_f32[0] * vec.m_f32[2] + m_columns[3].m_f32[0] * vec.m_f32[3],
            m_columns[0].m_f32[1] * vec.m_f32[0] + m_columns[1].m_f32[1] * vec.m_f32[1] + m_columns[2].m_f32[1] * vec.m_f32[2] + m_columns[3].m_f32[1] * vec.m_f32[3],
            m_columns[0].m_f32[2] * vec.m_f32[0] + m_columns[1].m_f32[2] * vec.m_f32[1] + m_columns[2].m_f32[2] * vec.m_f32[2] + m_columns[3].m_f32[2] * vec.m_f32[3],
            m_columns[0].m_f32[3] * vec.m_f32[0] + m_columns[1].m_f32[3] * vec.m_f32[1] + m_columns[2].m_f32[3] * vec.m_f32[2] + m_columns[3].m_f32[3] * vec.m_f32[3]
        );
#endif
    }

    Mat44 Mat44::operator*(const float scalar) const
    {
        const Vec4Reg multiplier = Vec4Reg::Replicate(scalar);

        Mat44 result;
        for (int col = 0; col < 4; ++col)
        {
            result.m_columns[col] = m_columns[col] * multiplier;
        }
        return result;
    }

    Mat44 operator*(const float scalar, const Mat44& mat)
    {
        return mat * scalar;
    }

    Mat44& Mat44::operator*=(const float scalar)
    {
        for (int col = 0; col < 4; ++col)
        {
            m_columns[col] *= scalar;
        }

        return *this;
    }

    Mat44 Mat44::operator+(const Mat44& other) const
    {
        Mat44 result;
        for (int col = 0; col < 4; ++col)
        {
            result.m_columns[col] = m_columns[col] + other.m_columns[col];
        }
        return result;
    }

    Mat44& Mat44::operator+=(const Mat44& other)
    {
        for (int col = 0; col < 4; ++col)
        {
            m_columns[col] += other.m_columns[col];
        }
        return *this;
    }

    Mat44 Mat44::operator-(const Mat44& other) const
    {
        Mat44 result;
        for (int col = 0; col < 4; ++col)
        {
            result.m_columns[col] = m_columns[col] - other.m_columns[col];
        }
        return result;
    }

    Mat44& Mat44::operator-=(const Mat44& other)
    {
        for (int col = 0; col < 4; ++col)
        {
            m_columns[col] -= other.m_columns[col];
        }
        return *this;
    }

    Mat44 Mat44::operator-() const
    {
        Mat44 result;
        for (int col = 0; col < 4; ++col)
        {
            result.m_columns[col] = -m_columns[col];
        }
        return result;
    }

    Mat44 Mat44::Identity()
    {
        return Mat44(
            Vec4Reg(1.f, 0.f, 0.f, 0.f),
            Vec4Reg(0.f, 1.f, 0.f, 0.f),
            Vec4Reg(0.f, 0.f, 1.f, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f));
    }

    Mat44 Mat44::Zero()
    {
        return Mat44(Vec4::Zero(), Vec4::Zero(), Vec4::Zero(), Vec4::Zero());
    }

    Mat44 Mat44::NaN()
    {
        return Mat44(Vec4::NaN(), Vec4::NaN(), Vec4::NaN(), Vec4::NaN());
    }

    Mat44 Mat44::LoadFloat4x4(const Float4* pFloats)
    {
        Mat44 result;
        for (int col = 0; col < 4; ++col)
        {
            result.m_columns[col] = Vec4::LoadFloat4(pFloats + col);
        }
        return result;
    }

    Mat44 Mat44::LoadFloat4x4Aligned(const Float4* pFloats)
    {
        Mat44 result;
        for (int col = 0; col < 4; ++col)
        {
            result.m_columns[col] = Vec4::LoadFloat4Aligned(pFloats + col);
        }
        return result;
    }

    void Mat44::StoreFloat4x4(Float4* pOutFloats) const
    {
        for (int col = 0; col < 4; ++col)
        {
            m_columns[col].StoreFloat4(pOutFloats + col);
        }
    }

    Mat44 Mat44::MakeRotationX(const float angle)
    {
        Vec4Reg sinV, cosV;
        Vec4Reg::Replicate(angle).SinCos(sinV, cosV);
        const float sin = sinV.GetX();
        const float cos = cosV.GetX();

        return Mat44
        (
            Vec4Reg(1.f, 0.f, 0.f, 0.f),
            Vec4Reg(0.f, cos, sin, 0.f),
            Vec4Reg(0.f, -sin, cos, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
    }

    Mat44 Mat44::MakeRotationY(const float angle)
    {
        Vec4Reg sinV, cosV;
        Vec4Reg::Replicate(angle).SinCos(sinV, cosV);
        const float sin = sinV.GetX();
        const float cos = cosV.GetX();

        return Mat44
        (
            Vec4Reg(cos, 0.f, -sin, 0.f),
            Vec4Reg(0.f, 1.f, 0.f, 0.f),
            Vec4Reg(sin, 0.f, cos, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
    }

    Mat44 Mat44::MakeRotationZ(const float angle)
    {
        Vec4Reg sinV, cosV;
        Vec4Reg::Replicate(angle).SinCos(sinV, cosV);
        const float sin = sinV.GetX();
        const float cos = cosV.GetX();

        return Mat44
        (
            Vec4Reg(cos, sin, 0.f, 0.f),
            Vec4Reg(-sin, cos, 0.f, 0.f),
            Vec4Reg(0.f, 0.f, 1.f, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
    }

    Mat44 Mat44::MakeRotation(const Vec3& axis, const float angle)
    {
        return MakeRotation(Quat::FromAxisAngle(axis, angle));
    }

    Mat44 Mat44::MakeRotation(const Quat& quat)
    {
        NES_ASSERT(quat.IsNormalized());

        const float x = quat.GetX();
        const float y = quat.GetY();
        const float z = quat.GetZ();
        const float w = quat.GetW();

        const float x2 = x + x;
        const float y2 = y + y;
        const float z2 = z + z;
        
        const float xx = x2 * x;
        const float xy = y2 * x;
        const float xz = z2 * x;
        const float yy = y2 * y;
        const float yz = z2 * y;
        const float zz = z2 * z;
        const float wx = w * x2;
        const float wy = w * y2;
        const float wz = w * z2;
        
        return Mat44
        (
            Vec4(1.f - (yy + zz), (xy + wz), (xz - wy), 0.f),
            Vec4(xy - wz, 1.f - (xx + zz), yz + wx, 0.f),
            Vec4(xz + wy, yz - wx, 1.f - (xx + yy), 0.f),
            Vec4(0.f, 0.f, 0.f, 1.f)
        );

        // See: https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation section 'Quaternion-derived rotation matrix'
    // #ifdef NES_USE_SSE4_1
    //     __m128 xyzw = quat.m_value.m_value;
    //     __m128 two_xyzw = _mm_add_ps(xyzw, xyzw);
    //     
    //     __m128 yzxw = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(3, 0, 2, 1));
    //     __m128 two_yzxw = _mm_add_ps(yzxw, yzxw);
    //     
    //     __m128 zxyw = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(3, 1, 0, 2));
    //     __m128 two_zxyw = _mm_add_ps(zxyw, zxyw);
    //     
    //     __m128 wwww = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(3, 3, 3, 3));
    //     __m128 diagonal = _mm_sub_ps(_mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(two_yzxw, yzxw)), _mm_mul_ps(two_zxyw, zxyw));	// (1 - 2 y^2 - 2 z^2, 1 - 2 x^2 - 2 z^2, 1 - 2 x^2 - 2 y^2, 1 - 4 w^2)
    //     
    //     //__m128 plus = _mm_add_ps(_mm_mul_ps(two_xyzw, zxyw), _mm_mul_ps(two_yzxw, wwww));										// 2 * (xz + yw, xy + zw, yz + xw, ww)
    //     __m128 plus = _mm_add_ps(_mm_mul_ps(two_xyzw, zxyw), _mm_mul_ps(two_yzxw, wwww));										// 2 * (xz + yw, xy + zw, yz + xw, ww)
    //     //__m128 minus = _mm_sub_ps(_mm_mul_ps(two_yzxw, xyzw), _mm_mul_ps(two_zxyw, wwww));
    //     __m128 minus = _mm_sub_ps(_mm_mul_ps(two_yzxw, xyzw), _mm_mul_ps(two_zxyw, wwww));
    //
    //     // Workaround for compiler changing _mm_sub_ps(_mm_mul_ps(...), ...) into a fused multiply sub instruction, resulting in w not being 0
    //     // There doesn't appear to be a reliable way to turn this off in Clang
    //     minus = _mm_insert_ps(minus, minus, 0b1000);
    //
    //     __m128 col0 = _mm_blend_ps(_mm_blend_ps(plus, diagonal, 0b0001), minus, 0b1100);	// (1 - 2 y^2 - 2 z^2, 2 xy + 2 zw, 2 xz - 2 yw, 0)
    //     __m128 col1 = _mm_blend_ps(_mm_blend_ps(diagonal, minus, 0b1001), plus, 0b0100);	// (2 xy - 2 zw, 1 - 2 x^2 - 2 z^2, 2 yz + 2 xw, 0)
    //     __m128 col2 = _mm_blend_ps(_mm_blend_ps(minus, plus, 0b0001), diagonal, 0b0100);	// (2 xz + 2 yw, 2 yz - 2 xw, 1 - 2 x^2 - 2 y^2, 0)
    //     
    //     //__m128 col0 = _mm_blend_ps(_mm_blend_ps(plus, diagonal, 0b0001), minus, 0b1010);	// (1 - 2 y^2 - 2 z^2, 2 xy + 2 zw, 2 xz - 2 yw, 0)
    //     //__m128 col1 = _mm_blend_ps(_mm_blend_ps(diagonal, minus, 0b1101), plus, 0b0001);	// (2 xy - 2 zw, 1 - 2 x^2 - 2 z^2, 2 yz + 2 xw, 0)
    //     //__m128 col2 = _mm_blend_ps(_mm_blend_ps(minus, plus, 0b0010), diagonal, 0b0100);	// (2 xz + 2 yw, 2 yz - 2 xw, 1 - 2 x^2 - 2 y^2, 0)
    //     
    //     __m128 col3 = _mm_set_ps(1, 0, 0, 0);
    //
    //     return Mat44(col0, col1, col2, col3);
    // #else
    //     const float x = quat.GetX();
    //     const float y = quat.GetY();
    //     const float z = quat.GetZ();
    //     const float w = quat.GetW();
    //
    //     const float tx = x + x; // Note: Using x + x instead of 2.0f * x to force this function to return the same value as the SSE4.1 version across platforms.
    //     const float ty = y + y;
    //     const float tz = z + z;
    //
    //     const float xx = tx * x;
    //     const float yy = ty * y;
    //     const float zz = tz * z;
    //     const float xy = tx * y;
    //     const float xz = tx * z;
    //     const float xw = tx * w;
    //     const float yz = ty * z;
    //     const float yw = ty * w;
    //     const float zw = tz * w;
    //
    //     return Mat44
    //     (
    //         Vec4((1.0f - yy) - zz, xy + zw, xz - yw, 0.0f), // Note: Added extra brackets to force this function to return the same value as the SSE4.1 version across platforms.
    //         Vec4(xy - zw, (1.0f - zz) - xx, yz + xw, 0.0f),
    //         Vec4(xz + yw, yz - xw, (1.0f - xx) - yy, 0.0f),
    //         Vec4(0.0f, 0.0f, 0.0f, 1.0f)
    //     );
    // #endif
    }

    Mat44 Mat44::MakeTranslation(const Vec3& translation)
    {
        return Mat44
        (
            Vec4Reg(1.f, 0.f, 0.f, 0.f),
            Vec4Reg(0.f, 1.f, 0.f, 0.f),
            Vec4Reg(0.f, 0.f, 1.f, 0.f),
            Vec4Reg(translation, 1.f)
        );
    }

    Mat44 Mat44::MakeRotationTranslation(const Quat& rotation, const Vec3& translation)
    {
        Mat44 result = MakeRotation(rotation);
        result.SetTranslation(translation);
        return result;
    }

    Mat44 Mat44::MakeInverseRotationTranslation(const Quat& rotation, const Vec3& translation)
    {
        Mat44 result = MakeRotation(rotation.Conjugate());
        result.SetTranslation(-result.Multiply3x3(translation));
        return result;
    }

    Mat44 Mat44::MakeScale(const float scale)
    {
        return Mat44
        (
            Vec4Reg(scale, 0.f, 0.f, 0.f),
            Vec4Reg(0.f, scale, 0.f, 0.f),
            Vec4Reg(0.f, 0.f, scale, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
    }

    Mat44 Mat44::MakeScale(const Vec3& scale)
    {
        return Mat44
        (
            Vec4Reg(scale.x, 0.f, 0.f, 0.f),
            Vec4Reg(0.f, scale.y, 0.f, 0.f),
            Vec4Reg(0.f, 0.f, scale.z, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
    }

    Mat44 Mat44::ComposeTransform(const Vec3& translation, const Quat& rotation, const Vec3& scale)
    {
        Mat44 result = MakeScale(scale);
        result = MakeRotationTranslation(rotation, translation) * result;
        return result;
    }

    Mat44 Mat44::ComposeTransform(const Vec3& translation, const Rotation& rotation, const Vec3& scale)
    {
        Quat quat = rotation.ToQuat();
        return ComposeTransform(translation, quat, scale);   
    }

    Mat44 Mat44::OuterProduct(const Vec3& a, const Vec3& b)
    {
        const Vec4Reg v1(a, 0.f);
        return Mat44
        (
            Vec4Reg(v1 * b.SplatX()),
            Vec4Reg(v1 * b.SplatY()),
            Vec4Reg(v1 * b.SplatZ()),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
    }

    Mat44 Mat44::CrossProduct(const Vec3& vec)
    {
#ifdef NES_USE_SSE4_1
        const Vec4Reg rVec3 = Vec4Reg(vec);
        
        // Zero out the W component
        const __m128 zero = _mm_setzero_ps();
        const __m128 v = _mm_blend_ps(rVec3.m_value, zero, 0b1000);

        // Negate
        const __m128 minV = _mm_sub_ps(zero, v);

        return Mat44
        (
            _mm_shuffle_ps(v, minV, _MM_SHUFFLE(3, 1, 2, 3)), // [0, z, -y, 0]
            _mm_shuffle_ps(minV, v, _MM_SHUFFLE(3, 0, 3, 2)), // [-z, 0, x, 0]
            _mm_blend_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 1)), _mm_shuffle_ps(minV, minV, _MM_SHUFFLE(3, 3, 0, 3)), 0b0010), // [y, -x, 0, 0]
            Vec4(0, 0, 0, 1)
        );
#else
        const float x = vec.x;
        const float y = vec.y;
        const float z = vec.z;

        return Mat44
        (
            Vec4Reg(0, z, -y, 0),
            Vec4Reg(-z, 0, x, 0),
            Vec4Reg(y, -x, 0, 0),
            Vec4Reg(0, 0, 0, 1)
        );
#endif
    }

    Mat44 Mat44::QuatLeftMultiply(const Quat& quat)
    {
        return Mat44
        (
            Vec4Reg(1, 1, -1, -1) * quat.m_value.Swizzle<ESwizzleW, ESwizzleZ, ESwizzleY, ESwizzleX>(),
            Vec4Reg(-1, 1, 1, -1) * quat.m_value.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleX, ESwizzleY>(),
            Vec4Reg(1, -1, 1, -1) * quat.m_value.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>(),
            quat.m_value
        );
    }

    Mat44 Mat44::QuatRightMultiply(const Quat& quat)
    {
        return Mat44
        (
            Vec4(1, -1, 1, -1) * quat.m_value.Swizzle<ESwizzleW, ESwizzleZ, ESwizzleY, ESwizzleX>(),
            Vec4(1, 1, -1, -1) * quat.m_value.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleX, ESwizzleY>(),
            Vec4(-1, 1, 1, -1) * quat.m_value.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>(),
            quat.m_value
        );
    }

    Mat44 Mat44::LookAt(const Vec3& eyePosition, const Vec3& target, const Vec3& upVector)
    {
        const Vec3 forward = (target - eyePosition).NormalizedOr(Vec3::AxisZ());
        const Vec3 right = upVector.Cross(forward).NormalizedOr(Vec3::AxisX());
        const Vec3 up = forward.Cross(right);

        Mat44 result = Mat44::Identity();
        result[0][0] = right.x;
        result[1][0] = right.y;
        result[2][0] = right.z;

        result[0][1] = up.x;
        result[1][1] = up.y;
        result[2][1] = up.z;

        result[0][2] = forward.x;
        result[1][2] = forward.y;
        result[2][2] = forward.z;

        result[3][0] = -right.Dot(eyePosition);
        result[3][1] = -up.Dot(eyePosition);
        result[3][2] = -forward.Dot(eyePosition);

        return result;
    }

    Mat44 Mat44::Perspective(const float fovRadians, const float aspectRatio, const float nearPlane, const float farPlane)
    {
        const float height = 1.f / math::Tan(0.5f * fovRadians);
        const float width = height / aspectRatio;
        
        Mat44 result = Zero();
        result.m_columns[0].m_f32[0] = height;
        result.m_columns[1].m_f32[1] = width;
        result.m_columns[2].m_f32[3] = 1.f;

#if NES_CLIP_VIEW_ZERO_TO_ONE
        result.m_columns[2].m_f32[2] = farPlane / (farPlane - nearPlane);
        result.m_columns[3].m_f32[2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
#else
        result.m_columns[2].m_f32[2] = (farPlane + nearPlane) / (farPlane - nearPlane);
        result.m_columns[3].m_f32[2] = -(2.f * farPlane * nearPlane) / (farPlane - nearPlane);
#endif
        return result;
    }

    Mat44 Mat44::Perspective(const float fovRadians, const float width, const float height, const float nearPlane, const float farPlane)
    {
        NES_ASSERT(width > 0.f);
        NES_ASSERT(height > 0.f);
        NES_ASSERT(fovRadians > 0.f);

        const float zoomY = math::Cos(0.5f * fovRadians) / math::Sin(0.5f * fovRadians);
        const float zoomX = zoomY * height / width;

        Mat44 result = Zero();
        result.m_columns[0].m_f32[0] = zoomX;
        result.m_columns[1].m_f32[1] = zoomY;
        result.m_columns[2].m_f32[3] = 1.f;

    #if NES_CLIP_VIEW_ZERO_TO_ONE
        result.m_columns[2].m_f32[2] = farPlane / (farPlane - nearPlane);
        result.m_columns[3].m_f32[2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
    #else
        result.m_columns[2].m_f32[2] = (farPlane + nearPlane) / (farPlane - nearPlane);
        result.m_columns[3].m_f32[2] = -(2.f * farPlane * nearPlane) / (farPlane - nearPlane);
    #endif
        return result;
    }

    Mat44 Mat44::Orthographic(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane)
    {
        Mat44 result = Identity();
        result.m_columns[0].m_f32[0] = 2.f / (right - left);
        result.m_columns[1].m_f32[1] = 2.f / (top - bottom);
        result.m_columns[3].m_f32[0] = -(right + left) / (right - left);
        result.m_columns[3].m_f32[1] = -(top + bottom) / (top - bottom);

    #if NES_CLIP_VIEW_ZERO_TO_ONE
        result.m_columns[2].m_f32[2] = 1.f / (farPlane - nearPlane);
        result.m_columns[3].m_f32[2] = -nearPlane / (farPlane - nearPlane);
    #else
        result.m_columns[2].m_f32[2] = 2.f / (farPlane - nearPlane);
        result.m_columns[3].m_f32[2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    #endif
        return result;
    }

    void Mat44::SetDiagonal3(const Vec3& diagonal)
    {
        m_columns[0][0] = diagonal.x;
        m_columns[1][1] = diagonal.y;
        m_columns[2][2] = diagonal.z;
    }

    void Mat44::SetDiagonal4(const Vec4& diagonal)
    {
        m_columns[0][0] = diagonal.x;
        m_columns[1][1] = diagonal.y;
        m_columns[2][2] = diagonal.z;
        m_columns[3][3] = diagonal.w;
    }

    void Mat44::SetColumn3(const uint column, const Vec3& val)
    {
        NES_ASSERT(column < 4);
        m_columns[column] = Vec4Reg(val, column == 3? 1.f : 0.f);
    }

    void Mat44::SetColumn4(const uint column, const Vec4& val)
    {
        NES_ASSERT(column < 4);
        m_columns[column] = val;
    }

    void Mat44::SetColumn4(const uint column, const Vec4Reg& val)
    {
        NES_ASSERT(column < 4);
        m_columns[column] = val;
    }

    Vec3 Mat44::GetRow3(const uint row) const
    {
        NES_ASSERT(row < 4);
        return Vec3(m_columns[0][row], m_columns[1][row], m_columns[2][row]);
    }

    void Mat44::SetRow3(const uint row, const Vec3& val)
    {
        NES_ASSERT(row < 4);    
        m_columns[0][row] = val.x;
        m_columns[1][row] = val.y;
        m_columns[2][row] = val.z;
    }

    Vec4 Mat44::GetRow4(const uint row) const
    {
        NES_ASSERT(row < 4);    
        return Vec4(m_columns[0][row], m_columns[1][row], m_columns[2][row], m_columns[3][row]);
    }
    
    void Mat44::SetRow4(const uint row, const Vec4& val)
    {
        NES_ASSERT(row < 4);    
        m_columns[0][row] = val.x;
        m_columns[1][row] = val.y;
        m_columns[2][row] = val.z;
        m_columns[3][row] = val.w;
    }

    bool Mat44::IsClose(const Mat44& other, const float maxSqrDist) const
    {
        for (int i = 0; i < 4; ++i)
        {
            if (!m_columns[i].IsClose(other.m_columns[i], maxSqrDist))
                return false;
        }

        return true;
    }

    Vec3 Mat44::Multiply3x3(const Vec3& vec) const
    {
    #if defined(NES_USE_SSE)
        const Vec4Reg rVec3 = Vec4Reg(vec);
        
        __m128 t = _mm_mul_ps(m_columns[0].m_value, _mm_shuffle_ps(rVec3.m_value, rVec3.m_value, _MM_SHUFFLE(0, 0, 0, 0)));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[1].m_value, _mm_shuffle_ps(rVec3.m_value, rVec3.m_value, _MM_SHUFFLE(1, 1, 1, 1))));
        t = _mm_add_ps(t, _mm_mul_ps(m_columns[2].m_value, _mm_shuffle_ps(rVec3.m_value, rVec3.m_value, _MM_SHUFFLE(2, 2, 2, 2))));
        return Vec4Reg(Vec4Reg::FixW(t)).ToVec3();
    #else
        return Vec3
        (
            m_columns[0].m_f32[0] * vec.x + m_columns[1].m_f32[0] * vec.y + m_columns[2].m_f32[0] * vec.z,
            m_columns[0].m_f32[1] * vec.x + m_columns[1].m_f32[1] * vec.y + m_columns[2].m_f32[1] * vec.z,
            m_columns[0].m_f32[2] * vec.x + m_columns[1].m_f32[2] * vec.y + m_columns[2].m_f32[2] * vec.z
        );
    #endif
    }

    Vec3 Mat44::Multiply3x3Transposed(const Vec3& vec) const
    {
    #if defined(NES_USE_SSE4_1)
        const Vec4Reg rVec3 = Vec4Reg(vec);
        
        const __m128 x = _mm_dp_ps(m_columns[0].m_value, rVec3.m_value, 0x7f);
        const __m128 y = _mm_dp_ps(m_columns[1].m_value, rVec3.m_value, 0x7f);
        const __m128 xy = _mm_blend_ps(x, y, 0b0010);
        const __m128 z = _mm_dp_ps(m_columns[2].m_value, rVec3.m_value, 0x7f);
        const __m128 xyzz = _mm_blend_ps(xy, z, 0b1100);
        return Vec4Reg(xyzz).ToVec3();
    #else
        return Transposed3x3().Multiply3x3(vec);
    #endif    
    }

    Mat44 Mat44::Multiply3x3(const Mat44& other) const
    {
        // Check that the bottom row is zeroed out.
        NES_ASSERT(m_columns[0][3] == 0.f);
        NES_ASSERT(m_columns[1][3] == 0.f);
        NES_ASSERT(m_columns[2][3] == 0.f);

        Mat44 result;
    #if defined (NES_USE_SSE)
        for (int i = 0; i < 3; ++i)
        {
            const __m128 c = other.m_columns[i].m_value;
            __m128 t = _mm_mul_ps(m_columns[0].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0)));
            t = _mm_add_ps(t, _mm_mul_ps(m_columns[1].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))));
            t = _mm_add_ps(t, _mm_mul_ps(m_columns[2].m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))));
            result.m_columns[i].m_value = t;
        }
    #else
        for (int i = 0; i < 3; ++i)
        {
            result.m_columns[i] = m_columns[0] * other.m_columns[i].m_f32[0] + m_columns[1] * other.m_columns[i].m_f32[1] + m_columns[2] * other.m_columns[i].m_f32[2];
        }
    #endif
        return result;
    }

    Mat44 Mat44::Multiply3x3LeftTransposed(const Mat44& other) const
    {
        // Transpose left hand side
        const Mat44 transposed = Transposed3x3();

        // Do a 3x3 multiply
        Mat44 result;
        result.m_columns[0] = (transposed.m_columns[0] * other.m_columns[0].SplatX()) + (transposed.m_columns[1] * other.m_columns[0].SplatY()) + (transposed.m_columns[2] * other.m_columns[0].SplatZ());
        result.m_columns[1] = (transposed.m_columns[0] * other.m_columns[1].SplatX()) + (transposed.m_columns[1] * other.m_columns[1].SplatY()) + (transposed.m_columns[2] * other.m_columns[1].SplatZ());
        result.m_columns[2] = (transposed.m_columns[0] * other.m_columns[2].SplatX()) + (transposed.m_columns[1] * other.m_columns[2].SplatY()) + (transposed.m_columns[2] * other.m_columns[2].SplatZ());
        result.m_columns[3] = Vec4(0.f, 0.f, 0.f, 1.f);
        return result;
    }

    Mat44 Mat44::Multiply3x3RightTransposed(const Mat44& other) const
    {
        // Make sure the bottom row is zeroed out.
        NES_ASSERT(m_columns[0][3] == 0.f);
        NES_ASSERT(m_columns[1][3] == 0.f);
        NES_ASSERT(m_columns[2][3] == 0.f);

        // Do a 3x3 multiply
        Mat44 result;
        result.m_columns[0] = (m_columns[0] * other.m_columns[0].SplatX()) + (m_columns[1] * other.m_columns[1].SplatX()) + (m_columns[2] * other.m_columns[2].SplatX());
        result.m_columns[1] = (m_columns[0] * other.m_columns[0].SplatY()) + (m_columns[1] * other.m_columns[1].SplatY()) + (m_columns[2] * other.m_columns[2].SplatY());
        result.m_columns[2] = (m_columns[0] * other.m_columns[0].SplatZ()) + (m_columns[1] * other.m_columns[1].SplatZ()) + (m_columns[2] * other.m_columns[2].SplatZ());
        result.m_columns[3] = Vec4(0.f, 0.f, 0.f, 1.f);
        return result;
    }

    Vec3 Mat44::TransformPoint(const Vec3& point) const
    {
        const Vec4Reg result = *this * Vec4Reg(point, 1.f);
        return result.ToVec3();
    }

    Vec2 Mat44::TransformPoint(const Vec2 point) const
    {
        const Vec3 result = Multiply3x3(Vec3(point.x, point.y, 1.f));
        return Vec2(result.x, result.y);
    }

    Mat44 Mat44::Transposed() const
    {
        Mat44 result;
    #if defined(NES_USE_SSE)
        const __m128 tmp1 = _mm_shuffle_ps(m_columns[0].m_value, m_columns[1].m_value, _MM_SHUFFLE(1, 0, 1, 0));
        const __m128 tmp3 = _mm_shuffle_ps(m_columns[0].m_value, m_columns[1].m_value, _MM_SHUFFLE(3, 2, 3, 2));
        const __m128 tmp2 = _mm_shuffle_ps(m_columns[2].m_value, m_columns[3].m_value, _MM_SHUFFLE(1, 0, 1, 0));
        const __m128 tmp4 = _mm_shuffle_ps(m_columns[2].m_value, m_columns[3].m_value, _MM_SHUFFLE(3, 2, 3, 2));

        result.m_columns[0].m_value = _mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(2, 0, 2, 0));
        result.m_columns[1].m_value = _mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(3, 1, 3, 1));
        result.m_columns[2].m_value = _mm_shuffle_ps(tmp3, tmp4, _MM_SHUFFLE(2, 0, 2, 0));
        result.m_columns[3].m_value = _mm_shuffle_ps(tmp3, tmp4, _MM_SHUFFLE(3, 1, 3, 1));
    #else
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                result.m_columns[row][col] = m_columns[col][row];
            }
        }
    #endif
        return result;
    }

    Mat44 Mat44::Transposed3x3() const
    {
        Mat44 result;
    #if defined (NES_USE_SSE)
        const __m128 zero = _mm_setzero_ps();
        const __m128 tmp1 = _mm_shuffle_ps(m_columns[0].m_value, m_columns[1].m_value, _MM_SHUFFLE(1, 0, 1, 0));
        const __m128 tmp3 = _mm_shuffle_ps(m_columns[0].m_value, m_columns[1].m_value, _MM_SHUFFLE(3, 2, 3, 2));
        const __m128 tmp2 = _mm_shuffle_ps(m_columns[2].m_value, zero, _MM_SHUFFLE(1, 0, 1, 0));
        const __m128 tmp4 = _mm_shuffle_ps(m_columns[2].m_value, zero, _MM_SHUFFLE(3, 2, 3, 2));
        
        result.m_columns[0].m_value = _mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(2, 0, 2, 0));
        result.m_columns[1].m_value = _mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(3, 1, 3, 1));
        result.m_columns[2].m_value = _mm_shuffle_ps(tmp3, tmp4, _MM_SHUFFLE(2, 0, 2, 0));
    #else
        for (int col = 0; col < 3; ++col)
        {
            for (int row = 0; row < 3; ++row)
            {
                result.m_columns[col].m_f32[row] = m_columns[row].m_f32[col];
            }
            result.m_columns[col].m_f32[3] = 0.f;
        }
    #endif
        result.m_columns[3] = Vec4(0.f, 0.f, 0.f, 1.f);
        return result;
    }

    Mat44 Mat44::Inversed() const
    {
    #if defined(NES_USE_SSE)
        // Algorithm from: http://download.intel.com/design/PentiumIII/sml/24504301.pdf
	    // Streaming SIMD Extensions - Inverse of 4x4 Matrix
	    // Adapted to load data using _mm_shuffle_ps instead of loading from memory
	    // Replaced _mm_rcp_ps with _mm_div_ps for better accuracy

	    __m128 tmp1 = _mm_shuffle_ps(m_columns[0].m_value, m_columns[1].m_value, _MM_SHUFFLE(1, 0, 1, 0));
	    __m128 row1 = _mm_shuffle_ps(m_columns[2].m_value, m_columns[3].m_value, _MM_SHUFFLE(1, 0, 1, 0));
	    __m128 row0 = _mm_shuffle_ps(tmp1, row1, _MM_SHUFFLE(2, 0, 2, 0));
	    row1 = _mm_shuffle_ps(row1, tmp1, _MM_SHUFFLE(3, 1, 3, 1));
	    tmp1 = _mm_shuffle_ps(m_columns[0].m_value, m_columns[1].m_value, _MM_SHUFFLE(3, 2, 3, 2));
	    __m128 row3 = _mm_shuffle_ps(m_columns[2].m_value, m_columns[3].m_value, _MM_SHUFFLE(3, 2, 3, 2));
	    __m128 row2 = _mm_shuffle_ps(tmp1, row3, _MM_SHUFFLE(2, 0, 2, 0));
	    row3 = _mm_shuffle_ps(row3, tmp1, _MM_SHUFFLE(3, 1, 3, 1));

	    tmp1 = _mm_mul_ps(row2, row3);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	    __m128 minor0 = _mm_mul_ps(row1, tmp1);
	    __m128 minor1 = _mm_mul_ps(row0, tmp1);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	    minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	    minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	    minor1 = _mm_shuffle_ps(minor1, minor1, _MM_SHUFFLE(1, 0, 3, 2));

	    tmp1 = _mm_mul_ps(row1, row2);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	    minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	    __m128 minor3 = _mm_mul_ps(row0, tmp1);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	    minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	    minor3 = _mm_shuffle_ps(minor3, minor3, _MM_SHUFFLE(1, 0, 3, 2));

	    tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, _MM_SHUFFLE(1, 0, 3, 2)), row3);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	    row2 = _mm_shuffle_ps(row2, row2, _MM_SHUFFLE(1, 0, 3, 2));
	    minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	    __m128 minor2 = _mm_mul_ps(row0, tmp1);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	    minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	    minor2 = _mm_shuffle_ps(minor2, minor2, _MM_SHUFFLE(1, 0, 3, 2));

	    tmp1 = _mm_mul_ps(row0, row1);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	    minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	    minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	    minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

	    tmp1 = _mm_mul_ps(row0, row3);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	    minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	    minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	    minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

	    tmp1 = _mm_mul_ps(row0, row2);
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	    minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	    tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	    minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

	    __m128 det = _mm_mul_ps(row0, minor0);
	    det = _mm_add_ps(_mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)), det); // Original code did (x + z) + (y + w), changed to (x + y) + (z + w) to match the ARM code below and make the result cross platform deterministic
	    det = _mm_add_ss(_mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)), det);
	    det = _mm_div_ss(_mm_set_ss(1.0f), det);
	    det = _mm_shuffle_ps(det, det, _MM_SHUFFLE(0, 0, 0, 0));

        Mat44 result;
	    result.m_columns[0].m_value = _mm_mul_ps(det, minor0);
	    result.m_columns[1].m_value = _mm_mul_ps(det, minor1);
	    result.m_columns[2].m_value = _mm_mul_ps(det, minor2);
	    result.m_columns[3].m_value = _mm_mul_ps(det, minor3);
        return result;
    #else
        float m00 = NES_RC(0, 0), m10 = NES_RC(1, 0), m20 = NES_RC(2, 0), m30 = NES_RC(3, 0);
	    float m01 = NES_RC(0, 1), m11 = NES_RC(1, 1), m21 = NES_RC(2, 1), m31 = NES_RC(3, 1);
	    float m02 = NES_RC(0, 2), m12 = NES_RC(1, 2), m22 = NES_RC(2, 2), m32 = NES_RC(3, 2);
	    float m03 = NES_RC(0, 3), m13 = NES_RC(1, 3), m23 = NES_RC(2, 3), m33 = NES_RC(3, 3);

	    float m10211120 = m10 * m21 - m11 * m20;
	    float m10221220 = m10 * m22 - m12 * m20;
	    float m10231320 = m10 * m23 - m13 * m20;
	    float m10311130 = m10 * m31 - m11 * m30;
	    float m10321230 = m10 * m32 - m12 * m30;
	    float m10331330 = m10 * m33 - m13 * m30;
	    float m11221221 = m11 * m22 - m12 * m21;
	    float m11231321 = m11 * m23 - m13 * m21;
	    float m11321231 = m11 * m32 - m12 * m31;
	    float m11331331 = m11 * m33 - m13 * m31;
	    float m12231322 = m12 * m23 - m13 * m22;
	    float m12331332 = m12 * m33 - m13 * m32;
	    float m20312130 = m20 * m31 - m21 * m30;
	    float m20322230 = m20 * m32 - m22 * m30;
	    float m20332330 = m20 * m33 - m23 * m30;
	    float m21322231 = m21 * m32 - m22 * m31;
	    float m21332331 = m21 * m33 - m23 * m31;
	    float m22332332 = m22 * m33 - m23 * m32;

	    const Vec4 col0(m11 * m22332332 - m12 * m21332331 + m13 * m21322231,		-m10 * m22332332 + m12 * m20332330 - m13 * m20322230,		m10 * m21332331 - m11 * m20332330 + m13 * m20312130,		-m10 * m21322231 + m11 * m20322230 - m12 * m20312130);
	    const Vec4 col1(-m01 * m22332332 + m02 * m21332331 - m03 * m21322231,		m00 * m22332332 - m02 * m20332330 + m03 * m20322230,		-m00 * m21332331 + m01 * m20332330 - m03 * m20312130,		m00 * m21322231 - m01 * m20322230 + m02 * m20312130);
	    const Vec4 col2(m01 * m12331332 - m02 * m11331331 + m03 * m11321231,		-m00 * m12331332 + m02 * m10331330 - m03 * m10321230,		m00 * m11331331 - m01 * m10331330 + m03 * m10311130,		-m00 * m11321231 + m01 * m10321230 - m02 * m10311130);
	    const Vec4 col3(-m01 * m12231322 + m02 * m11231321 - m03 * m11221221,		m00 * m12231322 - m02 * m10231320 + m03 * m10221220,		-m00 * m11231321 + m01 * m10231320 - m03 * m10211120,		m00 * m11221221 - m01 * m10221220 + m02 * m10211120);

	    float det = m00 * col0.x + m01 * col0.y + m02 * col0.z + m03 * col0.w;

	    return Mat44(col0 / det, col1 / det, col2 / det, col3 / det);
    #endif
    }

    Mat44 Mat44::InversedRotationTranslation() const
    {
        Mat44 result = Transposed3x3();
        result.SetTranslation(-result.Multiply3x3(GetTranslation()));
        return result;
    }

    float Mat44::Determinant3x3() const
    {
        return GetAxisX().Dot(GetAxisY().Cross(GetAxisZ()));   
    }

    float Mat44::Determinant() const
    {
        // This implementation is from pbrt.
        // Resources:
        // Page 162 of "3D Math Primer for Graphics and Game Development".
        // Page 27 of "Real-Time Collision Detection".
        const float s0 = m_columns[0][0] * m_columns[1][1] - m_columns[0][1] * m_columns[1][0];
        const float s1 = m_columns[0][0] * m_columns[2][1] - m_columns[0][1] * m_columns[2][0];
        const float s2 = m_columns[0][0] * m_columns[3][1] - m_columns[0][1] * m_columns[3][0];
        
        const float s3 = m_columns[1][0] * m_columns[2][1] - m_columns[1][1] * m_columns[2][0];
        const float s4 = m_columns[1][0] * m_columns[3][1] - m_columns[1][1] * m_columns[3][0];
        const float s5 = m_columns[2][0] * m_columns[3][1] - m_columns[2][1] * m_columns[3][0];
        
        const float c0 = m_columns[0][2] * m_columns[1][3] - m_columns[0][3] * m_columns[1][2];
        const float c1 = m_columns[0][2] * m_columns[2][3] - m_columns[0][3] * m_columns[2][2];
        const float c2 = m_columns[0][2] * m_columns[3][3] - m_columns[0][3] * m_columns[3][2];
        
        const float c3 = m_columns[1][2] * m_columns[2][3] - m_columns[1][3] * m_columns[2][2];
        const float c4 = m_columns[1][2] * m_columns[3][3] - m_columns[1][3] * m_columns[3][2];
        const float c5 = m_columns[2][2] * m_columns[3][3] - m_columns[2][3] * m_columns[3][2];
        
        return (s0 * c5) - ( s1 * c4)
            +  (s2 * c3) - (-s3 * c2)
            +  (s5 * c0) - ( s4 * c1);
    }

    Mat44 Mat44::Adjoint3x3() const
    {
        return Mat44
        (
            Vec4(NES_RC(1, 1), NES_RC(1, 2), NES_RC(1, 0), 0) * Vec4(NES_RC(2, 2), NES_RC(2, 0), NES_RC(2, 1), 0)
                - Vec4(NES_RC(1, 2), NES_RC(1, 0), NES_RC(1, 1), 0) * Vec4(NES_RC(2, 1), NES_RC(2, 2), NES_RC(2, 0), 0),
            Vec4(NES_RC(0, 2), NES_RC(0, 0), NES_RC(0, 1), 0) * Vec4(NES_RC(2, 1), NES_RC(2, 2), NES_RC(2, 0), 0)
                - Vec4(NES_RC(0, 1), NES_RC(0, 2), NES_RC(0, 0), 0) * Vec4(NES_RC(2, 2), NES_RC(2, 0), NES_RC(2, 1), 0),
            Vec4(NES_RC(0, 1), NES_RC(0, 2), NES_RC(0, 0), 0) * Vec4(NES_RC(1, 2), NES_RC(1, 0), NES_RC(1, 1), 0)
                - Vec4(NES_RC(0, 2), NES_RC(0, 0), NES_RC(0, 1), 0) * Vec4(NES_RC(1, 1), NES_RC(1, 2), NES_RC(1, 0), 0),
            Vec4(0, 0, 0, 1)
        );
    }

    Mat44 Mat44::Inversed3x3() const
    {
        const float det = Determinant3x3();

        return Mat44
        (
            (Vec4(NES_RC(1, 1), NES_RC(1, 2), NES_RC(1, 0), 0) * Vec4(NES_RC(2, 2), NES_RC(2, 0), NES_RC(2, 1), 0)
                - Vec4(NES_RC(1, 2), NES_RC(1, 0), NES_RC(1, 1), 0) * Vec4(NES_RC(2, 1), NES_RC(2, 2), NES_RC(2, 0), 0)) / det,
            (Vec4(NES_RC(0, 2), NES_RC(0, 0), NES_RC(0, 1), 0) * Vec4(NES_RC(2, 1), NES_RC(2, 2), NES_RC(2, 0), 0)
                - Vec4(NES_RC(0, 1), NES_RC(0, 2), NES_RC(0, 0), 0) * Vec4(NES_RC(2, 2), NES_RC(2, 0), NES_RC(2, 1), 0)) / det,
            (Vec4(NES_RC(0, 1), NES_RC(0, 2), NES_RC(0, 0), 0) * Vec4(NES_RC(1, 2), NES_RC(1, 0), NES_RC(1, 1), 0)
                - Vec4(NES_RC(0, 2), NES_RC(0, 0), NES_RC(0, 1), 0) * Vec4(NES_RC(1, 1), NES_RC(1, 2), NES_RC(1, 0), 0)) / det,
            Vec4(0, 0, 0, 1)
        );
    }

    bool Mat44::SetInversed3x3(const Mat44& matrix)
    {
        const float det = matrix.Determinant3x3();

        // If the determinant is zero the matrix is singular and we return false.
        if (det == 0.f)
            return false;

        *this = matrix.Adjoint3x3();
        m_columns[0] /= det;
        m_columns[1] /= det;
        m_columns[2] /= det;
        return true;
    }

    Mat44 Mat44::GetRotation() const
    {
        // Make sure the bottom row is zeroed out.
        NES_ASSERT(m_columns[0][3] == 0.0f);
        NES_ASSERT(m_columns[1][3] == 0.0f);
        NES_ASSERT(m_columns[2][3] == 0.0f);

        return Mat44(m_columns[0], m_columns[1], m_columns[2], Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    Mat44 Mat44::GetRotationSafe() const
    {
    #if defined(NES_USE_AVX512)
        return Mat44
        (
            _mm_maskz_mov_ps(0b0111, m_columns[0].m_value),
            _mm_maskz_mov_ps(0b0111, m_columns[1].m_value),
            _mm_maskz_mov_ps(0b0111, m_columns[2].m_value),
            Vec4(0, 0, 0, 1)
        );
    #elif defined(NES_USE_SSE4_1)
        const __m128 zero = _mm_setzero_ps();
        return Mat44
        (
            _mm_blend_ps(m_columns[0].m_value, zero, 8),
            _mm_blend_ps(m_columns[1].m_value, zero, 8),
            _mm_blend_ps(m_columns[2].m_value, zero, 8),
            Vec4(0, 0, 0, 1)
        );
    #else
        return Mat44
        (
            Vec4(m_columns[0].m_f32[0], m_columns[0].m_f32[1], m_columns[0].m_f32[2], 0),
            Vec4(m_columns[1].m_f32[0], m_columns[1].m_f32[1], m_columns[1].m_f32[2], 0),
            Vec4(m_columns[2].m_f32[0], m_columns[2].m_f32[1], m_columns[2].m_f32[2], 0),
            Vec4(0, 0, 0, 1)
        );
    #endif
    }

    void Mat44::SetRotation(const Mat44& rotation)
    {
        m_columns[0] = rotation.m_columns[0];
        m_columns[1] = rotation.m_columns[1];
        m_columns[2] = rotation.m_columns[2];
    }

    Quat Mat44::ToQuaternion() const
    {
        // pg 286 of "Math Primer for Graphics and Game Development".
        const float fourXSquaredMinus1 = m_columns[0].m_f32[0] - m_columns[1].m_f32[1] - m_columns[2].m_f32[2];
        const float fourYSquaredMinus1 = m_columns[1].m_f32[1] - m_columns[0].m_f32[0] - m_columns[2].m_f32[2];
        const float fourZSquaredMinus1 = m_columns[2].m_f32[2] - m_columns[0].m_f32[0] - m_columns[1].m_f32[1];
        const float fourWSquaredMinus1 = m_columns[0].m_f32[0] + m_columns[1].m_f32[1] + m_columns[2].m_f32[2];

        // Determine which of w, x, y, or z has the largest absolute value.
        int largestIndex = 0;
        float fourBiggerSquaredMinus1 = fourWSquaredMinus1;
        if (fourXSquaredMinus1 > fourBiggerSquaredMinus1)
        {
            fourBiggerSquaredMinus1 = fourXSquaredMinus1;
            largestIndex = 1;
        }

        if (fourYSquaredMinus1 > fourBiggerSquaredMinus1)
        {
            fourBiggerSquaredMinus1 = fourYSquaredMinus1;
            largestIndex = 2;
        }

        if (fourZSquaredMinus1 > fourBiggerSquaredMinus1)
        {
            fourBiggerSquaredMinus1 = fourZSquaredMinus1;
            largestIndex = 2;
        }

        const float largestValue = std::sqrt(fourBiggerSquaredMinus1 + 1.f) * 0.5f;
        const float mult = 0.25f / largestValue;

        switch (largestIndex)
        {
            case 0: // W
                return Quat
                (
                    (m_columns[1][2] - m_columns[2][1]) * mult,
                    (m_columns[2][0] - m_columns[0][2]) * mult,
                    (m_columns[0][1] - m_columns[1][0]) * mult,
                    largestValue
                );

            case 1: // X
                return Quat
                (
                    largestValue,
                    (m_columns[0][1] + m_columns[1][0]) * mult,
                    (m_columns[2][0] + m_columns[0][2]) * mult,
                    (m_columns[1][2] - m_columns[2][1]) * mult
                );

            case 2: // Y
                return Quat
                (
                    (m_columns[0][1] + m_columns[1][0]) * mult,
                    largestValue,
                    (m_columns[1][2] + m_columns[2][1]) * mult,
                    (m_columns[2][0] - m_columns[0][2]) * mult
                );

            case 3: // Z
                return Quat
                (
                    (m_columns[2][0] + m_columns[0][2]) * mult,
                    (m_columns[1][2] + m_columns[2][1]) * mult,
                    largestValue,
                    (m_columns[0][1] - m_columns[1][0]) * mult
                );

            default:
                NES_ASSERT(false);
                return Quat::Identity();
        }
    }

    Vec3 Mat44::GetScale() const
    {
        Vec3 result;
        [[maybe_unused]] auto rotationTranslation = Decompose(result);
        return result;
    }

    Mat44 Mat44::PreTranslated(const Vec3& translation) const
    {
        return Mat44(m_columns[0], m_columns[1], m_columns[2],
            Vec4(GetTranslation() + Multiply3x3(translation), 1.f));
    }

    Mat44 Mat44::PostTranslated(const Vec3& translation) const
    {
        return Mat44(m_columns[0], m_columns[1], m_columns[2],
            Vec4(GetTranslation() + translation, 1.f));
    }

    Mat44 Mat44::PreScaled(const Vec3& scale) const
    {
        return Mat44
        (
            scale.x * m_columns[0],
            scale.y * m_columns[1],
            scale.z * m_columns[2],
            m_columns[3]
        );
    }

    Mat44 Mat44::PostScaled(const Vec3& scale) const
    {
        const Vec4Reg scale4(scale, 1.f);
        return Mat44
        (
            scale4 * m_columns[0],
            scale4 * m_columns[1],
            scale4 * m_columns[2],
            scale4 * m_columns[3]
        );
    }

    Mat44 Mat44::Decompose(Vec3& outScale) const
    {
        // Start the modified Gram-Schmidt algorithm
        // X axis will just be normalized
        const Vec3 x = GetAxisX();

        // Make Y axis perpendicular to X
        Vec3 y = GetAxisY();
        const float xDotX = x.LengthSqr();
        y -= (x.Dot(y) / xDotX) * x;

        // Make Z axis perpendicular to X
        Vec3 z = GetAxisZ();
        z -= (x.Dot(z) / xDotX) * x;

        // Make Z axis perpendicular to Y
        const float yDotY = y.LengthSqr();
        z -= (y.Dot(z) / yDotY) * y;

        // Determine the scale
        const float zDotZ = z.LengthSqr();
        outScale = Vec3(xDotX, yDotY, zDotZ).Sqrt();

        // If the resulting x, y and z vectors don't form a left-handed matrix, flip the z axis.
        if (!Vec3::IsLeftHanded(x, y, z))
            outScale.z = -outScale.z;

        // Determine the rotation and translation
        return Mat44
        (
            Vec4Reg(x / outScale.x, 0),
            Vec4Reg(y / outScale.y, 0),
            Vec4Reg(z / outScale.z, 0),
            GetColumn4(3)
        );
    }

    void Mat44::Decompose(Vec3& outTranslation, Quat& outRotation, Vec3& outScale) const
    {
        const Mat44 rotationTranslation = Decompose(outScale);
        outTranslation = rotationTranslation.GetTranslation();
        outRotation = rotationTranslation.ToQuaternion();
    }

    inline void Mat44::Decompose(Vec3& outTranslation, Rotation& outRotation, Vec3& outScale) const
    {
        const Mat44 rotationTranslation = Decompose(outScale);
        outTranslation = rotationTranslation.GetTranslation();
        outRotation = Rotation(rotationTranslation.ToQuaternion().ToEulerAngles() * math::RadiansToDegrees<float>());
    }


#undef NES_RC
    
}