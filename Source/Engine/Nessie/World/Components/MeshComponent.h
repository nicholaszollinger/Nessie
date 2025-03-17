// MeshComponent.h
#pragma once
#include "Entity3DComponent.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/RendererContext.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]:
    //      - This should inherit from a Primitive Component for Collision aspects of the mesh.
    //      - There needs to be a mechanism for building render geometry from a set of vertices.
    //      - This should also be tied to a "scene proxy" which contains the current Render data, which
    //        only needs to be updated when the parent actor moves (to update the "proxy's" object matrix),
    //        or if Material parameters change.
    //     
    ///		@brief :  Base class for rendering 3D Geometry in the World.   
    //----------------------------------------------------------------------------------------------------
    class MeshComponent : public Entity3DComponent
    {
        NES_DEFINE_COMPONENT_TYPE(MeshComponent)
        
        std::shared_ptr<nes::RendererContext::GraphicsPipeline> m_pPipeline = nullptr;
        std::shared_ptr<Mesh> m_pMesh = nullptr;
        std::shared_ptr<Material> m_pMaterial = nullptr;

    public:
        virtual void PreRender() {}
        virtual void Render();
        
        void SetMaterial(const std::shared_ptr<Material>& pMaterial);
        [[nodiscard]] std::shared_ptr<Material>        GetMaterial();
        [[nodiscard]] const std::shared_ptr<Material>& GetMaterial() const;
        
        void SetPipeline(const std::shared_ptr<nes::RendererContext::GraphicsPipeline>& pipeline);
        [[nodiscard]] std::shared_ptr<nes::RendererContext::GraphicsPipeline> GetPipeline() const;

        void SetMesh(const std::shared_ptr<Mesh>& pMesh);
        [[nodiscard]] std::shared_ptr<Mesh> GetMesh() const;

    private:
        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
    };
}
