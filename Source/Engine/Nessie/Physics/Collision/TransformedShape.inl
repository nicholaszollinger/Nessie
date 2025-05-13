// TransformedShape.inl
#pragma once

namespace nes
{
    TransformedShape::TransformedShape(const Vector3& positionCOM, const Quat& rotation, const Shape* pShape, const BodyID& bodyID, const SubShapeIDCreator& subShapeIdCreator)
        : m_shapePositionCOM(positionCOM)
        , m_shapeRotation(rotation)
        , m_pShape(pShape)
        , m_bodyID(bodyID)
        , m_subShapeIDCreator(subShapeIdCreator)
    {
        //
    }

    void TransformedShape::SetWorldTransform(const Vector3& position, const Quat& rotation, const Vector3& scale)
    {
        m_shapePositionCOM = position + rotation * (scale * m_pShape->GetCenterOfMass());
        m_shapeRotation = rotation;
        SetShapeScale(scale);
    }

    void TransformedShape::SetWorldTransform(const Mat4& transform)
    {
        Vector3 scale;
        Mat4 rotationTranslation = transform.Decompose(scale);
        SetWorldTransform(rotationTranslation.GetColumn3(3), math::ToQuat(rotationTranslation), scale);
    }

    Mat4 TransformedShape::GetWorldTransform() const
    {
        Mat4 transform = math::MakeRotationMatrix4(m_shapeRotation).PreScaled(GetShapeScale());
        transform.SetTranslation(m_shapePositionCOM - transform.TransformVector(m_pShape->GetCenterOfMass()));
        return transform;
    }

    SubShapeID TransformedShape::MakeSubShapeIDRelativeToShape(const SubShapeID& subShapeID) const
    {
        SubShapeID result;
        const unsigned numBitsWritten = m_subShapeIDCreator.GetNumBitsWritten();
        
        NES_IF_LOGGING_ENABLED(const uint32_t rootID =)
        subShapeID.PopID(numBitsWritten, result);
        NES_ASSERT(rootID == (m_subShapeIDCreator.GetID().GetValue() & ((1 << numBitsWritten) - 1)));
        
        return result;
    }

    Vector3 TransformedShape::GetWorldSpaceSurfaceNormal(const SubShapeID& subShapeID, const Vector3& position) const
    {
        Mat4 inverseCOM = GetInverseCenterOfMassTransform();
        Vector3 scale = GetShapeScale();
        return inverseCOM.TransformVectorTranspose(m_pShape->GetSurfaceNormal(MakeSubShapeIDRelativeToShape(subShapeID), Vector3(inverseCOM.TransformPoint(position)) / scale) / scale).Normalized();
    }

    void TransformedShape::GetSupportingFace(const SubShapeID& subShapeID, const Vector3& direction,
        const Vector3& baseOffset, Shape::SupportingFace& outVertices) const
    {
        const Mat4 com = GetCenterOfMassTransform().PostTranslated(-baseOffset);
        m_pShape->GetSupportingFace(MakeSubShapeIDRelativeToShape(subShapeID), com.TransformVectorTranspose(direction), GetShapeScale(), com, outVertices);
    }

    uint64_t TransformedShape::GetSubShapeUserData(const SubShapeID& subShapeID) const
    {
        return m_pShape->GetSubShapeUserData(MakeSubShapeIDRelativeToShape(subShapeID));
    }

    TransformedShape TransformedShape::GetSubShapeTransformedShape(const SubShapeID& subShapeID, SubShapeID& outRemainder) const
    {
        TransformedShape result = m_pShape->GetSubShapeTransformedShape(subShapeID, Vector3::Zero(), m_shapeRotation, GetShapeScale(), outRemainder);
        result.m_shapePositionCOM += m_shapePositionCOM;
        return result;
    }
}