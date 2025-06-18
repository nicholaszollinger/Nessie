// TransformedShape.inl
#pragma once

namespace nes
{
    TransformedShape::TransformedShape(const Vec3& positionCOM, const Quat& rotation, const Shape* pShape, const BodyID& bodyID, const SubShapeIDCreator& subShapeIdCreator)
        : m_shapePositionCOM(positionCOM)
        , m_shapeRotation(rotation)
        , m_pShape(pShape)
        , m_bodyID(bodyID)
        , m_subShapeIDCreator(subShapeIdCreator)
    {
        //
    }

    void TransformedShape::SetWorldTransform(const Vec3& position, const Quat& rotation, const Vec3& scale)
    {
        m_shapePositionCOM = position + rotation * (scale * m_pShape->GetCenterOfMass());
        m_shapeRotation = rotation;
        SetShapeScale(scale);
    }

    void TransformedShape::SetWorldTransform(const Mat44& transform)
    {
        Vec3 scale;
        Mat44 rotationTranslation = transform.Decompose(scale);
        SetWorldTransform(rotationTranslation.GetColumn3(3), rotationTranslation.ToQuaternion(), scale);
    }

    Mat44 TransformedShape::GetWorldTransform() const
    {
        Mat44 transform = Mat44::MakeRotation(m_shapeRotation).PreScaled(GetShapeScale());
        transform.SetTranslation(m_shapePositionCOM - transform.TransformVector(m_pShape->GetCenterOfMass()));
        return transform;
    }

    SubShapeID TransformedShape::MakeSubShapeIDRelativeToShape(const SubShapeID& subShapeID) const
    {
        SubShapeID result;
        const unsigned numBitsWritten = m_subShapeIDCreator.GetNumBitsWritten();
        
        NES_IF_ASSERTS_ENABLED(const uint32_t rootID =)
        subShapeID.PopID(numBitsWritten, result);
        NES_ASSERT(rootID == (m_subShapeIDCreator.GetID().GetValue() & ((1 << numBitsWritten) - 1)));
        
        return result;
    }

    Vec3 TransformedShape::GetWorldSpaceSurfaceNormal(const SubShapeID& subShapeID, const Vec3& position) const
    {
        Mat44 inverseCOM = GetInverseCenterOfMassTransform();
        Vec3 scale = GetShapeScale();
        return inverseCOM.Multiply3x3Transposed(m_pShape->GetSurfaceNormal(MakeSubShapeIDRelativeToShape(subShapeID), Vec3(inverseCOM.TransformPoint(position)) / scale) / scale).Normalized();
    }

    void TransformedShape::GetSupportingFace(const SubShapeID& subShapeID, const Vec3& direction,
        const Vec3& baseOffset, Shape::SupportingFace& outVertices) const
    {
        const Mat44 com = GetCenterOfMassTransform().PostTranslated(-baseOffset);
        m_pShape->GetSupportingFace(MakeSubShapeIDRelativeToShape(subShapeID), com.Multiply3x3Transposed(direction), GetShapeScale(), com, outVertices);
    }

    uint64_t TransformedShape::GetSubShapeUserData(const SubShapeID& subShapeID) const
    {
        return m_pShape->GetSubShapeUserData(MakeSubShapeIDRelativeToShape(subShapeID));
    }

    TransformedShape TransformedShape::GetSubShapeTransformedShape(const SubShapeID& subShapeID, SubShapeID& outRemainder) const
    {
        TransformedShape result = m_pShape->GetSubShapeTransformedShape(subShapeID, Vec3::Zero(), m_shapeRotation, GetShapeScale(), outRemainder);
        result.m_shapePositionCOM += m_shapePositionCOM;
        return result;
    }
}