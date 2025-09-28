// MeshComponent.cpp
#include "MeshComponent.h"
#include "Nessie/World/World.h"
#include "Nessie/Graphics/Renderer.h"

namespace nes
{
    // void MeshComponent::Render()
    // {
    //     NES_ASSERT(false);
    //     
    //     // [TODO]: What about custom Shader Constants?
    //     // There should be a Material & MaterialInstance class that stores the push constant/uniform data
    //     // needed to Render the Mesh. m_pMaterial->SetConstant("ObjectMatrix", GetWorldTransformMatrix());
    //     // The Material would basically be the Pipeline object. How we render a set of vertices/indices in the World.
    //     // So all Materials would have a Vertex and Fragment shader, the Camera descriptor sets, the ObjectMatrix as a push constant
    //     // value at the very least.
    //
    //     //World::GeometryPushConstants pushConstants;
    //     //pushConstants.m_objectMatrix = GetOwner()->GetWorldTransformMatrix();
    //     //pushConstants.m_baseColor = m_pMaterial->m_baseColor;
    //     
    //     //Renderer::BindGraphicsPipeline(m_pPipeline);
    //     //Renderer::PushShaderConstant(m_pPipeline, vk::ShaderStageFlagBits::eVertex, 0, sizeof(World::GeometryPushConstants), &pushConstants);
    //     //Renderer::DrawIndexed(m_pMesh->GetVertexBuffer(), m_pMesh->GetIndexBuffer(), m_pMesh->GetIndexCount());
    // }
    //
    // void MeshComponent::SetMaterial(const std::shared_ptr<Material>& pMaterial)
    // {
    //     if (m_pMaterial == pMaterial)
    //         return;
    //
    //     if (m_pMaterial != nullptr)
    //     {
    //         // Release the Resource:
    //         // Use ExecuteCommands on Renderer to release?
    //     }
    //     
    //     m_pMaterial = pMaterial;
    //
    //     if (m_pMaterial == nullptr && IsEnabled())
    //     {
    //         // [TODO]: World->UnsubscribeToRender(this);
    //     }
    // }
    //
    // std::shared_ptr<Material> MeshComponent::GetMaterial()
    // {
    //     return m_pMaterial;
    // }
    //
    // const std::shared_ptr<Material>& MeshComponent::GetMaterial() const
    // {
    //     return m_pMaterial;
    // }
    //
    // // void MeshComponent::SetPipeline(const std::shared_ptr<nes::RendererContext::GraphicsPipeline>& pipeline)
    // // {
    // //     if (m_pPipeline == pipeline)
    // //         return;
    // //
    // //     if (m_pPipeline != nullptr)
    // //     {
    // //         // Release the Resource:
    // //         // Use ExecuteCommands on Renderer to release?
    // //     }
    // //
    // //     m_pPipeline = pipeline;
    // //     
    // //     // If our pipeline is now null, and we were being rendered, unsubscribe to World Render.
    // //     if (m_pMesh == nullptr && IsEnabled())
    // //     {
    // //         // [TODO]: World->UnsubscribeToRender(this);
    // //     }
    // // }
    //
    // // std::shared_ptr<nes::RendererContext::GraphicsPipeline> MeshComponent::GetPipeline() const
    // // {
    // //     return m_pPipeline;
    // // }
    //
    // void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& pMesh)
    // {
    //     if (m_pMesh == pMesh)
    //         return;
    //
    //     if (m_pMesh != nullptr)
    //     {
    //         // Release the Resource:
    //         // Use ExecuteCommands on Renderer to release?
    //     }
    //
    //     m_pMesh = pMesh;
    //
    //     // If our mesh is now null, and we were being rendered, unsubscribe to World Render.
    //     if (m_pMesh == nullptr && IsEnabled())
    //     {
    //         // [TODO]: World->UnsubscribeToRender(this);
    //     }
    // }
    //
    // std::shared_ptr<Mesh> MeshComponent::GetMesh() const
    // {
    //     return m_pMesh;
    // }
    //
    // void MeshComponent::OnEnabled()
    // {
    //     if (m_pMesh != nullptr)
    //     {
    //         GetOwner()->GetWorld()->RegisterMesh(this);
    //     }
    // }
    //
    // void MeshComponent::OnDisabled()
    // {
    //     // [TODO]: Unsubscribe to World Render
    // }
}
