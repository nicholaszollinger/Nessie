// ContactListenerImpl.h
#pragma once
#include "Nessie/Physics/Collision/ContactListener.h"

class ContactListenerImpl final : public nes::ContactListener
{
public:
    virtual nes::EValidateContactResult OnContactValidate(const nes::Body& body1, const nes::Body& body2, const nes::RVec3 baseOffset, const nes::CollideShapeResult& collisionResult) override;
    virtual void OnContactAdded(const nes::Body& body1, const nes::Body& body2, const nes::ContactManifold& manifold, nes::ContactSettings& ioSettings) override;
    virtual void OnContactPersisted(const nes::Body& body1, const nes::Body& body2, const nes::ContactManifold& manifold, nes::ContactSettings& ioSettings) override;
    virtual void OnContactRemoved(const nes::SubShapeIDPair& subShapePair) override;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Ability to defer to the next Contact Listener after this one handles the callback. 
    //----------------------------------------------------------------------------------------------------
    void SetNextListener(ContactListener* pListener) { m_pNext = pListener; }
    
private:
    ContactListener* m_pNext = nullptr;
};