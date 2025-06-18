// Mesh.h
#pragma once
#include <memory>
#include "Graphics/RenderAPI/Vulkan/Vulkan_Core.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //  [TODO]: Hide Vulkan behind proper Vertex Buffer and Index Buffer classes. 
    ///	@brief : Mesh Asset
    //----------------------------------------------------------------------------------------------------
    class Mesh
    {
    public:
        Mesh() = default;

        [[nodiscard]] vk::Buffer        GetVertexBuffer() const    { return m_vertexBuffer; }
        [[nodiscard]] vk::Buffer        GetIndexBuffer() const     { return m_indexBuffer; }
        uint32_t                        GetVertexCount() const     { return m_vertexCount; }
        uint32_t                        GetIndexCount() const      { return m_indexCount; }
        
        static std::shared_ptr<Mesh>    Create(const void* pVertexData, const size_t vertexSize, const uint32_t vertexCount, const std::vector<uint32_t>& indices);
        static std::shared_ptr<Mesh>    Create(const void* pVertexData, const size_t vertexSize, const uint32_t vertexCount, const void* pIndices, const size_t indexSize, const uint32_t indexCount);
        static void                     Free(Mesh& mesh);

    private:
        vk::Buffer  m_vertexBuffer = nullptr;
        vk::Buffer  m_indexBuffer = nullptr;
        uint32_t    m_vertexCount = 0;
        uint32_t    m_indexCount = 0;
    };
}