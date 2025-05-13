// Transform.h
#pragma once
#include "Matrix.h"

namespace nes
{
    template <FloatingPointType Type>
    struct TTransform3
    {
        TQuaternion<Type> m_orientation = TQuaternion<Type>::Identity();
        TVector3<Type> m_location{};
        TVector3<Type> m_scale = Vector3::Unit();
        
        constexpr TTransform3() = default;
        constexpr TTransform3(const TVector3<Type>& location, const TQuaternion<Type>& orientation, const TVector3<Type>& scale);

        void Rotate(const Type angle, const TVector3<Type>& axis);
        void Rotate(const TQuaternion<Type>& deltaRotation);
        void Translate(const TVector3<Type>& deltaTranslation);
        void Scale(const TVector3<Type>& scale);
        void Scale(const Type uniformScale);
        
        [[nodiscard]] TMatrix4x4<Type> ToMatrix() const;
    };

    using Transform = TTransform3<NES_PRECISION_TYPE>;
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TTransform3<Type>::TTransform3(const TVector3<Type>& location, const TQuaternion<Type>& orientation,
       const TVector3<Type>& scale)
        : m_orientation(orientation)
        , m_location(location)
        , m_scale(scale)
    {
        //
    }

    template <FloatingPointType Type>
    void TTransform3<Type>::Rotate(const Type angle, const TVector3<Type>& axis)
    {
        TQuaternion<Type> rotation = TQuaternion<Type>::MakeFromAngleAxis(angle, axis);
        m_orientation = rotation * m_orientation;
    }

    template <FloatingPointType Type>
    void TTransform3<Type>::Rotate(const TQuaternion<Type>& deltaRotation)
    {
        m_orientation = deltaRotation * m_orientation;
    }

    template <FloatingPointType Type>
    void TTransform3<Type>::Translate(const TVector3<Type>& deltaTranslation)
    {
        m_location += deltaTranslation;
    }

    template <FloatingPointType Type>
    void TTransform3<Type>::Scale(const TVector3<Type>& scale)
    {
        m_scale *= scale;
    }

    template <FloatingPointType Type>
    void TTransform3<Type>::Scale(const Type uniformScale)
    {
        m_scale *= uniformScale;
    }
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> TTransform3<Type>::ToMatrix() const
    {
        const TMatrix4x4<Type> scale = math::MakeScaleMatrix(m_scale);
        const TMatrix4x4<Type> rotation = math::ToMat4(m_orientation);
        const TMatrix4x4<Type> translation = math::MakeTranslationMatrix4(m_location);
        return translation * rotation * scale;
    }
}

