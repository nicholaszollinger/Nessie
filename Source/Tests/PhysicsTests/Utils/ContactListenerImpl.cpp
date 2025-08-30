// ContactListenerImpl.cpp
#include "ContactListenerImpl.h"
#include "Nessie/Physics/Body/Body.h"
#include "Nessie/Physics/Collision/CollideShape.h"
#include "Nessie/Core/QuickSort.h"

nes::EValidateContactResult ContactListenerImpl::OnContactValidate(const nes::Body& body1, const nes::Body& body2, const nes::RVec3 baseOffset, const nes::CollideShapeResult& collisionResult)
{
    // Check ordering contract between body1 and body2
    [[maybe_unused]] const bool contract = body1.GetMotionType() >= body2.GetMotionType()
        || (body1.GetMotionType() == body2.GetMotionType() && body1.GetID() < body2.GetID());

    NES_ASSERT(contract);
    
    nes::EValidateContactResult result;
    if (m_pNext != nullptr)
        result = m_pNext->OnContactValidate(body1, body2, baseOffset, collisionResult);
    else
        result = ContactListener::OnContactValidate(body1, body2, baseOffset, collisionResult);

    // [TODO]: Debug render an arrow.
    //nes::RVec3 contactPoint = baseOffset + collisionResult.m_contactPointOn1;
    NES_TRACE("Validate {} and {}, result: {}", body1.GetID().GetIndex(), body2.GetID().GetIndex(), static_cast<int>(result));
    
    return result;
}

void ContactListenerImpl::OnContactAdded(const nes::Body& body1, const nes::Body& body2, const nes::ContactManifold& manifold, nes::ContactSettings& ioSettings)
{
    NES_ASSERT(body1.GetID() < body2.GetID());
    NES_TRACE("Contact added between {} ({:#x}) and {} ({:#x})", body1.GetID().GetIndex(), manifold.m_subShapeID1.GetValue(), body2.GetID().GetIndex(), manifold.m_subShapeID2.GetValue());
    
    // [TODO]: Debug render the contact.

    if (m_pNext != nullptr)
        m_pNext->OnContactAdded(body1, body2, manifold, ioSettings);
}

void ContactListenerImpl::OnContactPersisted(const nes::Body& body1, const nes::Body& body2, const nes::ContactManifold& manifold, nes::ContactSettings& ioSettings)
{
    NES_ASSERT(body1.GetID() < body2.GetID());
    NES_TRACE("Contact persisted between {} ({:#x}) and {} ({:#x})", body1.GetID().GetIndex(), manifold.m_subShapeID1.GetValue(), body2.GetID().GetIndex(), manifold.m_subShapeID2.GetValue());
    
    // [TODO]: Debug render the contact.

    if (m_pNext != nullptr)
        m_pNext->OnContactPersisted(body1, body2, manifold, ioSettings);
}

void ContactListenerImpl::OnContactRemoved(const nes::SubShapeIDPair& subShapePair)
{
    NES_ASSERT(subShapePair.GetBody1ID() < subShapePair.GetBody2ID());
    NES_TRACE("Contact removed between {} ({:#x}) and {} ({:#x})", subShapePair.GetBody1ID().GetIndex(), subShapePair.GetSubShape1ID().GetValue(), subShapePair.GetBody2ID().GetIndex(), subShapePair.GetSubShape2ID().GetValue());

    if (m_pNext != nullptr)
        m_pNext->OnContactRemoved(subShapePair);
}