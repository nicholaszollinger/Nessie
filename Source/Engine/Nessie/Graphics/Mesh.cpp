// Mesh.cpp
#include "Mesh.h"
#include "Renderer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a new Mesh Asset with a set of Vertex and index data. 
    ///		@param pVertexData : Array of vertices.
    ///		@param vertexSize : Size of the vertex object.
    ///		@param vertexCount : Number of Vertices.
    ///		@param indices : Array of indices that indicate how to draw the mesh.
    ///		@returns : shared_ptr to the new Mesh Asset.
    //----------------------------------------------------------------------------------------------------
    std::shared_ptr<Mesh> Mesh::Create(const void* pVertexData, const size_t vertexSize, const uint32_t vertexCount,
                                       const std::vector<uint32_t>& indices)
    {
        auto pMesh = std::make_shared<Mesh>();
        pMesh->m_vertexBuffer = Renderer::CreateVertexBuffer(pVertexData, vertexSize, vertexCount);
        pMesh->m_vertexCount = vertexCount;
        pMesh->m_indexBuffer = Renderer::CreateIndexBuffer(indices);
        pMesh->m_indexCount = static_cast<uint32_t>(indices.size());
        return pMesh;
    }

    std::shared_ptr<Mesh> Mesh::Create(const void* pVertexData, const size_t vertexSize, const uint32_t vertexCount,
        const void* pIndices, const size_t indexSize, const uint32_t indexCount)
    {
        auto pMesh = std::make_shared<Mesh>();
        pMesh->m_vertexBuffer = Renderer::CreateVertexBuffer(pVertexData, vertexSize, vertexCount);
        pMesh->m_vertexCount = vertexCount;
        pMesh->m_indexBuffer = Renderer::CreateIndexBuffer(pIndices, indexSize, indexCount);
        pMesh->m_indexCount = indexCount;
        return pMesh;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Free a Mesh Resource. 
    //----------------------------------------------------------------------------------------------------
    void Mesh::Free(Mesh& mesh)
    {
        Renderer::DestroyBuffer(mesh.m_vertexBuffer);
        Renderer::DestroyBuffer(mesh.m_indexBuffer);
    }
}