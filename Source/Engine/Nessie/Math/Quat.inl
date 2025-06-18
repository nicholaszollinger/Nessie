// Quat.inl
#pragma once

namespace nes
{
    Quat Quat::operator*(const Quat& other) const
    {
    #if defined (NES_USE_SSE4_1)
        // Taken from: http://momchil-velikov.blogspot.nl/2013/10/fast-sse-quternion-multiplication.html
        __m128 abcd = Vec4Reg::LoadVec4(&m_value).m_value;
        __m128 xyzw = Vec4Reg::LoadVec4(&other.m_value).m_value;

        __m128 t0 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(3, 3, 3, 3));
        __m128 t1 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(2, 3, 0, 1));

        __m128 t3 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(0, 0, 0, 0));
        __m128 t4 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(1, 0, 3, 2));

        __m128 t5 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 t6 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(2, 0, 3, 1));

        // [d,d,d,d] * [z,w,x,y] = [dz,dw,dx,dy]
        __m128 m0 = _mm_mul_ps(t0, t1);

        // [a,a,a,a] * [y,x,w,z] = [ay,ax,aw,az]
        __m128 m1 = _mm_mul_ps(t3, t4);

        // [b,b,b,b] * [z,x,w,y] = [bz,bx,bw,by]
        __m128 m2 = _mm_mul_ps(t5, t6);

        // [c,c,c,c] * [w,z,x,y] = [cw,cz,cx,cy]
        __m128 t7 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(2, 2, 2, 2));
        __m128 t8 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(3, 2, 0, 1));
        __m128 m3 = _mm_mul_ps(t7, t8);

        // [dz,dw,dx,dy] + -[ay,ax,aw,az] = [dz+ay,dw-ax,dx+aw,dy-az]
        __m128 e = _mm_addsub_ps(m0, m1);

        // [dx+aw,dz+ay,dy-az,dw-ax]
        e = _mm_shuffle_ps(e, e, _MM_SHUFFLE(1, 3, 0, 2));

        // [dx+aw,dz+ay,dy-az,dw-ax] + -[bz,bx,bw,by] = [dx+aw+bz,dz+ay-bx,dy-az+bw,dw-ax-by]
        e = _mm_addsub_ps(e, m2);

        // [dz+ay-bx,dw-ax-by,dy-az+bw,dx+aw+bz]
        e = _mm_shuffle_ps(e, e, _MM_SHUFFLE(2, 0, 1, 3));

        // [dz+ay-bx,dw-ax-by,dy-az+bw,dx+aw+bz] + -[cw,cz,cx,cy] = [dz+ay-bx+cw,dw-ax-by-cz,dy-az+bw+cx,dx+aw+bz-cy]
        e = _mm_addsub_ps(e, m3);

        // [dw-ax-by-cz,dz+ay-bx+cw,dy-az+bw+cx,dx+aw+bz-cy]
        Vec4Reg result(_mm_shuffle_ps(e, e, _MM_SHUFFLE(2, 3, 1, 0)));
        return Quat(result.GetX(), result.GetY(), result.GetZ(), result.GetW());
    #else
        const float lx = m_value.x;
        const float ly = m_value.y;
        const float lz = m_value.z;
        const float lw = m_value.w;

        const float rx = other.m_value.x;
        const float ry = other.m_value.y;
        const float rz = other.m_value.z;
        const float rw = other.m_value.w;

        const float x = lw * rx + lx * rw + ly * rz - lz * ry;
        const float y = lw * ry - lx * rz + ly * rw + lz * rx;
        const float z = lw * rz + lx * ry - ly * rx + lz * rw;
        const float w = lw * rw - lx * rx - ly * ry - lz * rz;

        return Quat(x, y, z, w);
    #endif
    }

    Vec3 Quat::operator*(const Vec3& vec) const
    {
        // Rotating a vector by a quaternion is done by: p' = q * p * q^-1 (q^-1 = conjugated(q) for a unit quaternion)
        NES_ASSERT(IsNormalized());
        return Vec3((*this * Quat(Vec4(vec, 0.f)) * Conjugate()).m_value);
    }

    void Quat::ToAxisAngle(Vec3& outAxis, float& outAngle) const
    {
        NES_ASSERT(IsNormalized());
        const Quat wPositive = EnsureWPositive();
        const float absW = wPositive.GetW();
        if (absW >= 1.0f)
        {
            outAxis = Vec3::Zero();
            outAngle = 0.0f;
        }
        else
        {
            outAngle = 2.0f * math::ACos(absW);
            outAxis = wPositive.GetXYZ().NormalizedOr(Vec3::Zero());
        }
    }

    Quat Quat::FromTo(const Vec3& from, const Vec3& to)
    {
        /*
        Uses (inFrom = v1, inTo = v2):

        angle = arcos(v1 . v2 / |v1||v2|)
        axis = normalize(v1 x v2)

        Quaternion is then:

        s = sin(angle / 2)
        x = axis.x * s
        y = axis.y * s
        z = axis.z * s
        w = cos(angle / 2)

        Using identities:

        sin(2 * a) = 2 * sin(a) * cos(a)
        cos(2 * a) = cos(a)^2 - sin(a)^2
        sin(a)^2 + cos(a)^2 = 1

        This reduces to:

        x = (v1 x v2).x
        y = (v1 x v2).y
        z = (v1 x v2).z
        w = |v1||v2| + v1 . v2

        which then needs to be normalized because the whole equation was multiplied by 2 cos(angle / 2)
        */

        const float lenV1V2 = std::sqrt(from.LengthSqr() * to.LengthSqr());
        const float w = lenV1V2 + from.Dot(to);

        if (w == 0.f)
        {
            if (lenV1V2 == 0.f)
            {
                // If either of the vectors has zero lengths, there is no rotation and we return identity.
                return Quat::Identity();
            }

            // If the vectors are perpendicular, take one of the many 180-degree rotations that exist.
            return Quat(Vec4(from.NormalizedPerpendicular(), 0.f));
        }

        const Vec3 vec = from.Cross(to);
        return Quat(Vec4(vec, w)).Normalized();
    }

    Quat Quat::EulerAngles(const Vec3& eulerAngles)
    {
        const Vec4Reg half(0.5f * eulerAngles);
        Vec4Reg sin, cos;
        half.SinCos(sin, cos);
        
        const float cx = cos.GetX();
        const float cy = cos.GetY();
        const float cz = cos.GetZ();
        
        const float sx = sin.GetX();
        const float sy = sin.GetY();
        const float sz = sin.GetZ();

        return Quat
        (
            (sx * cy * cz) - (cx * sy * sz),
            (cx * sy * cz) + (sx * cy * sz),
            (cx * cy * sz) - (sx * sy * cz),
            (cx * cy * cz) + (sx * sy * sz)
        );
    }

    Vec3 Quat::ToEulerAngles() const
    {
        const float ySqr = GetY() * GetY();

        // X
        const float t0 = 2.f * (GetW() * GetX() + GetY() * GetZ());
        const float t1 = 1.0f - 2.0f * (GetX() * GetX() + ySqr);

        // Y
        float t2 = 2.0f * (GetW() * GetY() - GetZ() * GetX());
        t2 = t2 > 1.0f? 1.0f : t2;
        t2 = t2 < -1.0f? -1.0f : t2;

        // Z
        const float t3 = 2.0f * (GetW() * GetZ() + GetX() * GetY());
        const float t4 = 1.0f - 2.0f * (ySqr + GetZ() * GetZ());

        return Vec3(math::ATan2(t0, t1), math::ASin(t2), math::ATan2(t3, t4));
    }

    Quat& Quat::Normalize()
    {
        *this = Normalized();
        return *this;
    }

    Vec3 Quat::InverseRotate(const Vec3& vec) const
    {
        NES_ASSERT(IsNormalized());
        return Vec3((Conjugate() * Quat(Vec4(vec, 0.f)) * *this).m_value);
    }

    Vec3 Quat::RotateAxisX() const
    {
        // This is *this * Vec3::AxisX() written out:
        NES_ASSERT(IsNormalized());

        const float x = GetX();
        const float y = GetY();
        const float z = GetZ();
        const float w = GetW();
        const float tx = 2.f * x;
        const float tw = 2.f * w;
        
        return Vec3(tx * x + tw * w - 1.0f, tx * y + z * tw, tx * z - y * tw);
    }

    Vec3 Quat::RotateAxisY() const
    {
        // This is *this * Vec3::AxisY() written out:
        NES_ASSERT(IsNormalized());

        const float x = GetX();
        const float y = GetY();
        const float z = GetZ();
        const float w = GetW();
        const float ty = 2.f * y;
        const float tw = 2.f * w;
        
        return Vec3(x * ty - z * tw, tw * w + ty * y - 1.0f, x * tw + ty * z);
    }

    Vec3 Quat::RotateAxisZ() const
    {
        // This is *this * Vec3::AxisZ() written out:
        NES_ASSERT(IsNormalized());

        const float x = GetX();
        const float y = GetY();
        const float z = GetZ();
        const float w = GetW();
        const float tz = 2.f * z;
        const float tw = 2.f * w;
        
        return Vec3(x * tz + y * tw, y * tz - x * tw, tw * w + tz * z - 1.0f);
    }

    float Quat::Pitch() const
    {
        const float x = GetX();
        const float y = GetY();
        const float z = GetZ();
        const float w = GetW();
        
        const float sinPitch = 2.f * ((y * z) + (w * x));
        const float cosPitch = (w * w) - (x * x) - (y * y) + (z * z);
        
        // Handle potential Gimbal Lock.
        if ((sinPitch == 0.f) && (cosPitch == 0.f))
        {
            return 2.f * math::ATan2(x, w);
        }
        
        return math::ATan2(sinPitch, cosPitch);
    }

    float Quat::Yaw() const
    {
        return math::ASin(-2.f * ((GetX() * GetZ()) - (GetW() * GetY())));
    }

    float Quat::Roll() const
    {
        const float x = GetX();
        const float y = GetY();
        const float z = GetZ();
        const float w = GetW();
        
        const float sinRoll = 2.f * ((x * y) + (w * z));
        const float cosRoll = (w * w) + (x * x) - (y * y) - (z * z);
        
        // Handle potential Gimbal Lock.
        if ((sinRoll == 0.f) && (cosRoll == 0.f))
        {
            return 0.f;
        }
        
        return math::ATan2(sinRoll, cosRoll);
    }

    Quat Quat::Conjugate() const
    {
        //return Quat(Vec4::Xor(m_value, UVec4Reg(0x80000000, 0x80000000, 0x80000000, 0).ReinterpretAsFloat()));
        return Quat(-GetX(), -GetY(), -GetZ(), GetW());
    }

    Quat Quat::Inverse() const
    {
        return Conjugate() / Length();
    }

    Quat& Quat::Invert()
    {
        *this = Inverse();
        return *this;
    }

    Quat Quat::EnsureWPositive() const
    {
        const auto val = Vec4Reg::LoadVec4(&m_value);
        const auto w = m_value.SplatW();

        const auto result = Vec4Reg::Xor(val, Vec4Reg::And(w, UVec4Reg::Replicate(0x80000000).ReinterpretAsFloat()));
        return Quat(result.ToVec4());
    }

    Quat Quat::GetPerpendicular() const
    {
        return Quat(Vec4(1, -1, 1, -1) * m_value.Swizzle<ESwizzleY, ESwizzleX, ESwizzleW, ESwizzleZ>());
    }

    float Quat::GetRotationAngle(const Vec3& axis) const
    {
        return GetW() == 0.f? math::Pi<float>() : 2.f * math::ATan(GetXYZ().Dot(axis) / GetW());
    }

    Quat Quat::GetTwist(const Vec3& axis) const
    {
        Quat twist(Vec4(GetXYZ().Dot(axis) * axis, GetW()));
        const float twistLength = twist.LengthSqr();
        if (twistLength != 0.0f)
            return twist / sqrt(twistLength);
        else
            return Quat::Identity();
    }

    void Quat::GetSwingTwist(Quat& outSwing, Quat& outTwist) const
    {
        const float x = GetX();
        const float y = GetY();
        const float z = GetZ();
        const float w = GetW();
        const float s = sqrt(math::Squared(w) + math::Squared(x));
        
        if (s != 0.f)
        {
            outTwist = Quat(x / s, 0, 0, w / s);
            outSwing = Quat(0, (w * y - x * z) / s, (w * z + x * y) / s, s);
        }
        else
        {
            // If both x and w are zero, this must be a 180-degree rotation around either y or z.
            outTwist = Quat::Identity();
            outSwing = *this;
        }
    }

    Quat Quat::Lerp(const Quat& destination, const float fraction) const
    {
        const float scale0 = 1.f - fraction;
        return Quat(Vec4::Replicate(scale0) * m_value + Vec4::Replicate(fraction) * destination.m_value);
    }

    Quat Quat::Lerp(const Quat& start, const Quat& end, const float fraction)
    {
        const float scale0 = 1.f - fraction;
        return Quat(Vec4::Replicate(scale0) * start.m_value + Vec4::Replicate(fraction) * end.m_value);
    }

    Quat Quat::Slerp(const Quat& destination, const float fraction) const
    {
        // Difference at which to Lerp instead of Slerp.
        constexpr float kDelta = 0.0001f;

        // Calculate cosine.
        float signScale1 = 1.f;
        float cosOmega = Dot(destination);

        // Adjust sings (if necessary)
        if (cosOmega < 0.f)
        {
            cosOmega = -cosOmega;
            signScale1 = -1.f;
        }

        // Calculate coefficients
        float scale0, scale1;
        if (1.f - cosOmega > kDelta)
        {
            // Standard case, slerp
            float omega = math::ACos(cosOmega);
            float sinOmega = math::Sin(omega);
            scale0 = math::Sin((1.f - fraction) * omega) / sinOmega;
            scale1 = signScale1 * math::Sin(fraction * omega) / sinOmega;
        }
        else
        {
            // Quaternions are very close, lerp instead
            scale0 = 1.f - fraction;
            scale1 = signScale1 * fraction;
        }

        // Interpolate between the two quaternions.
        return Quat(Vec4::Replicate(scale0) * m_value + Vec4::Replicate(scale1) * destination.m_value).Normalized();
    }

    Quat Quat::Slerp(const Quat& start, const Quat& end, const float fraction)
    {
        return start.Slerp(end, fraction);
    }

    Quat Quat::LoadFloat3Unsafe(const Float3& value)
    {
        const Vec3 vec = Vec3::LoadFloat3Unsafe(value);
        // It is possible that the length of v is a fraction above 1, and we don't want to introduce NaN's in that case, so we clamp to 0.
        const float w = std::sqrt(math::Max(1.f - vec.LengthSqr(), 0.f));
        return Quat(Vec4(vec, w));
    }

    void Quat::StoreFloat3(Float3* outValue) const
    {
        NES_ASSERT(IsNormalized());
        EnsureWPositive().GetXYZ().StoreFloat3(outValue);
    }

    Quat Quat::FromAxisAngle(const Vec3& axis, const float angle)
    {
        // returns [inAxis * sin(0.5f * inAngle), cos(0.5f * inAngle)]
        NES_ASSERT(axis.IsNormalized());

        Vec4Reg sin, cos;
        Vec4Reg::Replicate(0.5f * angle).SinCos(sin, cos);
        const auto result = Vec4Reg::Select(Vec4Reg(axis) * sin, cos, UVec4Reg(0, 0, 0, 0xffffffffU));
        return Quat(result.ToVec4());
    }
}