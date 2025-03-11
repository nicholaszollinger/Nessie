// AbstractLayer.cpp
#include "AbstractLayer.h"

namespace nes
{
    AbstractLayer::AbstractLayer(Scene* pScene)
        : EntityLayer(pScene)
        , m_entityPool(this)
    {
        //
    }

    void AbstractLayer::DestroyEntity(const LayerHandle& handle)
    {
        m_entityPool.QueueDestroyEntity(handle);
    }

    bool AbstractLayer::IsValidEntity(const LayerHandle& handle) const
    {
        return m_entityPool.IsValidEntity(handle);
    }

    bool AbstractLayer::InitializeLayer()
    {
        return true;
    }

    void AbstractLayer::OnSceneBegin()
    {
        //
    }
    
    void AbstractLayer::DestroyLayer()
    {
        m_entityPool.ClearPool();
    }

    bool AbstractLayer::LoadLayer([[maybe_unused]] YAML::Node& layerNode)
    {
        // [TODO]:
        // This is largely going to be the same as the world. I am just looking for an identifier, creating an Entity
        // of the Layer's Type, then Adding the Components as I encounter them. This logic needs to be pushed into
        // the base class if possible.
        return true;
    }

    void AbstractLayer::RenderEditorEntityHierarchy()
    {
        // [TODO]: 
    }
}
