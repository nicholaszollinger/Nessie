// Pipeline.h
#pragma once

//-------------------------------------------------------------------------------------------------
// Under development. Only here to preserve work that I may come back to.
//-------------------------------------------------------------------------------------------------

namespace nes
{
    // enum class EPipelineBlendMode : uint8
    // {
    //     Additive,   /// Add colors together when blending (outColor = srcColor.rgb * srcColor.a + dstColor.rgb * 1.0).
    //     AlphaBlend, /// Mix the colors based on their alpha value of the new color (outColor = srcColor.rgb * srcColor.a + dstColor.rgb * (1.0 - srcColor.a)).
    // };
    //
    // enum class EPrimitiveTopology : uint8
    // {
    //     PointList = 0,
    //     LineList,
    //     LineStrip,
    //     TriangleList,
    //     TriangleStrip,
    //     TriangleFan,
    // };
    //
    // enum class EPolygonMode : uint8
    // {
    //     Fill,   /// Polygon surfaces should be colored in.
    //     Line,   /// Just vertex edges should be rendered. "WireFrame".
    //     Point,  /// Just vertices themselves should be rendered.
    // };
    //
    // enum class ECullMode : uint8
    // {
    //     None,
    //     Front,
    //     Back,
    //     FrontAndBack,
    // };
    //
    // enum class EFrontFace : uint8
    // {
    //     CounterClockwise,
    //     Clockwise,
    // };
    //
    // //----------------------------------------------------------------------------------------------------
    // /// @brief : Defines a Shader object to use at a particular shader in the pipeline.
    // //----------------------------------------------------------------------------------------------------
    // struct ShaderStageInfo
    // {
    //     EShaderStage        m_stages;
    //     StrongPtr<Shader>   m_pShader;
    // };
    //
    // //----------------------------------------------------------------------------------------------------
    // /// @brief : Defines a small block of data to be used within a shader. Push constants are easy to send
    // /// shaders data, but only in a limited size (usually about ~256 bytes for all push constants).
    // /// Generally, you should be using Shader Uniforms, but for small, simple stuff, push constants can be a great tool.
    // /// @note : Push constant data needs to be pushed every frame.
    // //----------------------------------------------------------------------------------------------------
    // struct PushConstantLayout
    // {
    //     EShaderStage        m_targetStage;  /// Which stages should receive this push constant data?
    //     uint32              m_size;         /// Size of the data sent to the push constant.
    //     uint32              m_offset;       /// Offset in the entire push constant block for this data.  
    //
    //     PushConstantLayout(const EShaderStage stage, const uint32 size, const uint32 offset = 0) : m_targetStage(stage), m_size(size), m_offset(offset) {}
    // };
    //
    // class Pipeline : public RefTarget<Pipeline>
    // {
    // public:
    //     virtual void SetPushConstantData(const void* pData, const uint32 size, const uint32 offset = 0) = 0;
    // };
    //
    // //----------------------------------------------------------------------------------------------------
    // // [TODO]:
    // // - Dynamic States
    // // - Viewport & Scissor = Always Dynamic.
    // // - Logic Op for color blend state.
    // // - Shader Modules
    // // - Shader Uniforms
    // // - Target frame buffer / Render pass.
    //
    // /// @brief : Class to build Pipeline assets.
    // ///     All settings functions return a reference to the instance ("return *this"), so that you can 
    // ///     chain together function calls.
    // //----------------------------------------------------------------------------------------------------
    // class PipelineBuilder
    // {
    // public:
    //     virtual ~PipelineBuilder() = default;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Set the type of primitive topology that this pipeline operates on. This defines how to 
    //     ///     operate on an array of vertices to build a primitive.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    SetInputTopology(const EPrimitiveTopology topology) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Set the shaders to use for this pipeline.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    SetShaders(const std::vector<ShaderStageInfo>& shaderStages) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Set any push constant layouts that you want to use for the pipeline. Push constant data must
    //     ///     be set every frame.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    SetPushConstantLayouts(const std::vector<PushConstantLayout>& pushConstants) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Determines what pixels are sent to the fragment shader: The entire triangle surface,
    //     ///     the triangle edges, etc.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    SetPolygonMode(const EPolygonMode mode, const float lineWidth = 1.f) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Set which sides of the face should be culled, and set the winding order of the vertices to
    //     ///     determine a front vs. back face.
    //     ///	@param cullMode : Determine which side of the faces should be culled when occluded.
    //     ///	@param frontFace : Determines the "winding" order in which vertices are stored in the vertex buffer.  
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    SetCullMode(const ECullMode cullMode, const EFrontFace frontFace) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Determine how to blend color between overlapping pixels.  
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    EnableBlending(const EPipelineBlendMode blendMode) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Determine how to blend color between overlapping pixels, with finer control.
    //     ///	@param srcFactor : How to calculate the source color.
    //     ///	@param dstFactor : How to calculate the destination color.
    //     ///	@param colorOp : Formula to use for color blending. 
    //     ///	@param alphaOp : Formula to use for alpha blending. 
    //     ///	@param writeMask : Which color components can you write to.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    EnableBlending(const EBlendFactor srcFactor, const EBlendFactor dstFactor, const EBlendOperation colorOp, const EBlendOperation alphaOp, const EColorComponent writeMask);
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Disable blending for this pipeline.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    DisableBlending() = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Enable depth testing for this pipeline.
    //     ///	@param enableWrite : Should we write our true when the compare op returns true? 
    //     ///	@param compareOp : How to compare the dest depth value vs. the src depth value.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    EnableDepthTest(const bool enableWrite, const ECompareOp compareOp) = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Disable depth testing for this pipeline.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    DisableDepthTest() = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Disable multisampling for this pipeline.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual PipelineBuilder&    DisableMultisampling() = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     ///	@returns : Build the Pipeline asset from the set settings. 
    //     //----------------------------------------------------------------------------------------------------
    //     virtual StrongPtr<Pipeline> Build() = 0;
    //
    //     //----------------------------------------------------------------------------------------------------
    //     /// @brief : Reset all settings to their default state.
    //     /// @note : This should be called in the constructor of derived classes to initialize the Builder object.
    //     //----------------------------------------------------------------------------------------------------
    //     virtual void                Reset() = 0;
    // };
    
    
}