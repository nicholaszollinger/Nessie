// RenderContext.h
#pragma once
#include "Core/Config.h"
#ifdef NES_RENDER_API_VULKAN
#include "Graphics/RenderAPI/Vulkan/Vulkan_Core.h"

#include <functional>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

///------------------------------------------------------------------------
/// VULKAN WRAPPER FOR GAP-311
/// Written by Josh Stefanski.
///
/// This file contains a wrapper around some core Vulkan objects and
/// some related helpers to make working with the Vulkan API a bit less
/// verbose and confusing. It is not intended to be a feature-complete
/// abstraction layer, instead just a light wrapper. For many advanced
/// usage cases it should not get in your way / you are free to manually
/// take over. The method-chaining pattern of member initialization
/// is used extensively in this code as coupled with Vulkan-Hpp it
/// can improve the readability of the larger structs and leverage some
///	features to handle simpler code.
///------------------------------------------------------------------------

/// Enable hashing of some vulkan handles so we can use them with hashed containers
template<> struct std::hash<vk::Pipeline> : std::hash<VkPipeline> {};
template<> struct std::hash<vk::PipelineLayout> : std::hash<VkPipelineLayout> {};

namespace nes::internal
{
    VKAPI_ATTR VkBool32 VKAPI_CALL DefaultDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
}

namespace nes
{
    class Window;
    struct ApplicationProperties;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : This structure represents a light abstraction of the major components of a
    ///             vk::GraphicsPipelineCreateInfo structure. 
    //----------------------------------------------------------------------------------------------------
    struct GraphicsPipelineConfig
    {
        //-----------------------------------------------------------------------------------------------------------
        /// VERTEX INPUTS
        ///
        /// Input Bindings and Input Attributes define aspects of what are referred to as
        ///	per-vertex inputs. In the typical sense vertex inputs define how we send a polygonal
        ///	mesh to the GPU for rendering. The GPU will act on each of these inputs, often in
        ///	a massively-parallel environment. But how we store the data in system memory might
        ///	differ from how the GPU would like it, so we use these descriptions to describe
        ///	how the different components of a vertex (like position, normal, color, UV, etc.)
        ///	are laid out in the vertex buffer(s) that will be used.
        //-----------------------------------------------------------------------------------------------------------
        
        /// A Vertex Input Binding defines a slot (called a binding) that a vertex buffer can be
        ///	plugged into during rendering. The binding defines the rate of input (per-vertex most commonly)
        ///	and the stride between each vertex in the buffer. Think of this as defining aspects of a
        ///	for loop if you were to loop over the buffer yourself. There will always be at least one binding.
        std::vector<vk::VertexInputBindingDescription> m_vertexBindings = {};

        /// A Vertex Input Attribute defines a component of a vertex -- what binding it is in, what byte
        ///	offset to apply to get to the data, and the format of the data itself.
        /// A simple pattern for defining a vertex in system memory might be to make a struct that represents
        ///	each vertex and have a member variable for each component of the vertex. This would then correlate
        ///	to a single binding that has a stride equal to the size of the struct and a vertex attribute for
        ///	each member variable, using its offset and type.
        std::vector<vk::VertexInputAttributeDescription> m_vertexAttributes = {};

        /// The last part of defining vertex inputs is to indicate how the vertexes correllate with
        ///	each other to form a geometric primitive. Most commonly these are triangles of some form
        ///	either in a list or a strip, the latter being a way to define adjacent triangles in a compact fashion.
        /// 
        vk::PrimitiveTopology m_topology = vk::PrimitiveTopology::eTriangleList;

        //-----------------------------------------------------------------------------------------------------------
        /// UNIFORM INPUTS
        ///
        /// Uniform (also known as constant) inputs are inputs to shaders that do not change
        /// during a draw call. So these values are "uniformly constant" across all shader program
        ///	invocations in the drawing operation. Contrast this to vertex or stream inputs, which
        ///	change for each invocation of a shader.
        ///
        /// Push constants and descriptor sets are two ways of providing uniformly constant data.
        //-----------------------------------------------------------------------------------------------------------

        /// Push constants are constant values that are inserted directly into the command buffer
        ///	using the vk::CommandBuffer::pushConstants function prior to the draw command.
        ///	This is a mechanism for providing more granular constant data than a normal uniform buffer.
        ///	An example application is to pass in the object's transform matrix via a push constant block.
        ///
        std::vector<vk::PushConstantRange> m_shaderPushConstants = {};

        /// Descriptor sets are groupings of constant data provided to shaders. The provided data can be
        ///	raw buffers, images, or other more complex types.
        ///	Layouts are declarations -- mostly the size and format of the data. So providing the layouts
        ///	here says that when we bind the pipeline for usage, we expect descriptor sets (the actual data)
        ///	to be bound as well, filling in these layout "slots"
        ///	
        std::vector<vk::DescriptorSetLayout> m_shaderUniforms = {};

        //-----------------------------------------------------------------------------------------------------------
        /// SHADER PROGRAMS
        //-----------------------------------------------------------------------------------------------------------

        /// This is where the code for each shader stage gets supplied. Only the vertex and fragment (pixel) stages
        ///	are typically required, but other stages can be added as needed.
        ///	The order specified here does not matter.
        ///
        std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages = {};

        //-----------------------------------------------------------------------------------------------------------
        /// RASTERIZATION
        ///
        /// Rasterization state dictates how we take the geometric primitives and map them to color fragments (pixels)
        ///	that will be passed to the fragment shader.
        //-----------------------------------------------------------------------------------------------------------

        /// Polygon mode dictates if we're filling our polygon, drawing only the edges, or even just the vertex points themselves.
        vk::PolygonMode m_polygonMode = vk::PolygonMode::eFill;

        /// Cull mode and front face designation work together to perform an operation common to optimization
        ///	and other rendering techniques: choosing whether or not to draw faces pointing away from the viewport.
        vk::CullModeFlags m_cullMode = vk::CullModeFlagBits::eNone;
        
        ///	Front face describes the winding -- order of vertexes -- that defines if the triangle is facing forward or backwards.
        ///	Cull mode controls which directions will be skipped when rasterizing.
        vk::FrontFace m_frontFace = vk::FrontFace::eCounterClockwise;

        //-----------------------------------------------------------------------------------------------------------
        /// DEPTH AND STENCIL
        ///
        /// The depth and stencil state controls how the depth and stencil buffer operations are performed.
        ///	Not all render passes have depth/stencil buffers, so these operations are only valid if a depth/stencil buffer is used.
        ///
        ///	Depth testing is used as a way to skip fragment (pixel) options by performing depth-based comparisons for the fragments
        ///	that are generated. This can end up as an optimization (not processing a color fragment that is occluded by another) or
        ///	as part of a rendering technique (checking against a light's depth map for shadow mapping)
        //-----------------------------------------------------------------------------------------------------------
        
        ///	Tests are only performed if enabled and a fragment is considered to pass the depth test if the comparison operation between
        ///	its depth and the existing depth value at the same location evaluates to true.
        vk::Bool32 m_depthTestEnable = false;
        vk::CompareOp m_depthCompareOp = vk::CompareOp::eNever;

        /// For some techniques (like drawing semitransparent objects) you might want to not write the value that passes the test back
        ///	to the depth buffer, so that can be controlled separately.
        vk::Bool32 m_depthWriteEnable = false;

        //-----------------------------------------------------------------------------------------------------------
        /// BLENDING
        //-----------------------------------------------------------------------------------------------------------

        /// Once we have shaded a color fragment (or pixel) we must now incorporate it into the framebuffer that we are drawing into.
        /// A sensible default (provided below) just writes out the color value directly. But this is also where we can configure
        ///	how semitransparent objects can be blended with the scene through a technique known as alpha blending.
        std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendStates =
        {
	        // By default allow writing of all components
	        vk::PipelineColorBlendAttachmentState()
		        .setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
        };

        //-----------------------------------------------------------------------------------------------------------
        /// OUTPUT
        ///
        /// The output stage defines where and into what you are drawing. This is where the results of the graphics pipeline end up.
        ///
        /// As part of the output stages, the regions of the framebuffer that are being drawn into need to be defined.
        ///	Most of the time you will have one viewport and one scissor
        //-----------------------------------------------------------------------------------------------------------
        
        ///	A viewport defines the region, in pixel coordinates, that is being rendered into. When drawing to the screen
        ///	this is most commonly the screen resolution. Rendering will be scaled to fit in the viewport.
        std::vector<vk::Viewport> m_viewports = {};

        /// A scissor rectangle defines a further constraint within the viewport.
        ///	Most commonly the scissor dimensions will match the viewport.
        std::vector<vk::Rect2D> m_scissors = {};

        /// Finally, a render pass is an object that represents the formats of the output data being generated and into which
        ///	color attachments of the framebuffer the data should be written into.
        ///
        /// NOTE: This can be left null if using with VulkanWrapper::CreatePipeline()
        ///
        vk::RenderPass m_renderPass;

        //-----------------------------------------------------------------------------------------------------------
        /// DYNAMIC STATE
        ///
        /// For optimization reasons, Vulkan prefers knowing as much about the intended state
        /// of the pipeline ahead of time. But this isn't always desired or sometimes possible,
        ///	so there is a mechanism called Dynamic State which allows us to set parts of the pipeline
        ///	state directly in the command buffer before we issue the draw call.
        ///	To use this, we tell Vulkan what states it should expect to be set later by providing it
        ///	an array of state identifiers.
        //-----------------------------------------------------------------------------------------------------------
        std::vector<vk::DynamicState> m_dynamicStates = {};
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      For now, this class is essentially using a Vulkan Wrapper class that was supplied by my graphics class.
    //      I want the data in this class to be housed in separate objects (A Swapchain object for example).
    //      This class should hold the main components, but the objects themselves have the functionality.
    //		
    ///		@brief : 
    //----------------------------------------------------------------------------------------------------
    class RendererContext
    {
    public:
        using DebugLogFunc = std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*)>;
        using ConfigureInstanceFunc = std::function<void(vkb::InstanceBuilder&)>;
        using ConfigureDeviceFunc = std::function<void(vkb::PhysicalDeviceSelector&)>;
        
        struct ConfigOptions
        {
            std::string m_applicationName = "App";

            // Vulkan requires upper limits to be defined when it comes to both the
            // maximum number of descriptor sets (values plugged into a shader) as well as
            // the types of those sets. These are some reasonable defaults for the narrow
            // scope of the course.
            std::vector<vk::DescriptorPoolSize> m_descriptorPoolSizes =
            {
                { vk::DescriptorType::eUniformBuffer, 128 },
                { vk::DescriptorType::eCombinedImageSampler, 128 },
            };
            uint32_t m_maxDescriptorSets = 256;
            
            // For simplification the wrapper turns off depth/stencil support by default.
            bool m_enableDepthStencilBuffer = false;

            // To refine the Vulkan Device or Instance creation process, provide callbacks here
            ConfigureDeviceFunc m_configureDeviceFunc;
            ConfigureInstanceFunc m_configureInstanceFunc;
            
            // To enable debugging and validation, provide a logging callback
            // A framework-specific helper might be available for this, see the VulkanLogTo* methods
            DebugLogFunc m_debugLogFunc;
        };

        struct GraphicsPipeline : std::tuple<vk::Pipeline, vk::PipelineLayout, GraphicsPipelineConfig>
        {
            vk::Pipeline                  GetPipeline() const { return std::get<0>(*this); }
            vk::PipelineLayout            GetLayout()   const { return std::get<1>(*this); }
            const GraphicsPipelineConfig& GetConfig()   const { return std::get<2>(*this); }

            operator vk::Pipeline()       const { return GetPipeline(); }
            operator vk::PipelineLayout() const { return GetLayout(); }
        };
        
        struct ShaderUniform : std::tuple<vk::DescriptorSet, vk::DescriptorSetLayout>
        {
            vk::DescriptorSet       GetSet()    const { return std::get<0>(*this); }
            vk::DescriptorSetLayout GetLayout() const { return std::get<1>(*this); }

            operator vk::DescriptorSet()       const { return GetSet(); }
            operator vk::DescriptorSetLayout() const { return GetLayout(); }
        };
        
        struct RenderTarget
        {
            std::vector<vk::Image> m_images{};
            std::vector<vk::ImageView> m_views{};
            vk::Framebuffer m_framebuffer{};
            vk::RenderPass m_renderPass{};
        };

    public:
        inline static const DebugLogFunc s_defaultDebugLogFunction = internal::DefaultDebugCallback;

    public:
        RendererContext() = default;

        // No Copy or Move.
        RendererContext(const RendererContext&) = delete;
        RendererContext& operator=(const RendererContext&) = delete;
        RendererContext(RendererContext&&) = delete;
        RendererContext& operator=(RendererContext&&) = delete;

    private:
        std::unordered_map<VkBuffer, vk::DeviceMemory> m_bufferMemoryMap;
        std::unordered_map<VkImage, vk::DeviceMemory> m_imageMemoryMap;
        std::vector<std::weak_ptr<GraphicsPipeline>> m_graphicsPipelines;
        DebugLogFunc m_debugLogFunc;
        
        vkb::Instance       m_vkbInstance;
        vkb::Device         m_vkbDevice;
        vkb::PhysicalDevice m_vkbPhysicalDevice;
        vkb::Swapchain      m_vkbSwapchain;

        vk::SurfaceKHR      m_displaySurface;
        vk::Viewport        m_displayViewport;
        vk::RenderPass      m_displayRenderPass;

        vk::Format          m_depthFormat{};
        vk::ImageView       m_depthStencilView;
        vk::Image           m_depthStencilImage;

        vk::CommandPool     m_graphicsCommandPool;
        vk::DescriptorPool  m_descriptorPool;
        vk::PipelineCache   m_pipelineCache;

        vk::Queue           m_graphicsQueue;
        vk::Queue           m_presentQueue;

        struct FramebufferData
        {
            vk::Framebuffer m_framebuffer{};
            vk::ImageView m_imageView{};
            vk::CommandBuffer m_commandBuffer{};
            vk::Fence m_inUse{};
        };
        
        std::vector<FramebufferData> m_framebuffers;
        uint32_t m_currentFramebufferIndex = 0;

        static constexpr uint32_t kMaxPendingFrames = 2;
        struct FrameSyncStatus
        {
            vk::Semaphore m_isImageAvailable{};
            vk::Semaphore m_isRenderFinished{};
            vk::Fence m_inUse{};
        };
        FrameSyncStatus m_frames[kMaxPendingFrames];
        uint32_t m_currentFrameIndex = 0;
        
    public:
        bool Init(Window* pWindow, const ApplicationProperties& props, const ConfigOptions& options);
        void Shutdown();

        //-----------------------------------------------------------------------------------------------------------
        // COMMAND EXECUTION
        //
        // These methods provide means of queueing and executing commands that are stored in a command buffer.
        //-----------------------------------------------------------------------------------------------------------
        bool BeginFrame(vk::CommandBuffer& commandBuffer, vk::Framebuffer& framebuffer);
        void EndFrame();
        bool ExecuteCommands(std::function<void(vk::CommandBuffer&)>&& generateCommands);
        vk::CommandBuffer CreateSecondaryCommandBuffer();
        
        //-----------------------------------------------------------------------------------------------------------
        // BUFFER RESOURCES
        //
        // Buffers are effectively arrays or blobs of memory for usage by the GPU.
        //	Vulkan separates memory allocation from resource creation and so these methods will store
        //	the memory object so it does not need to be directly managed. Because of this, the DestroyBuffer
        //	method must be used with CreateBuffer.
        //-----------------------------------------------------------------------------------------------------------
        [[nodiscard]] vk::Buffer CreateBuffer(const vk::BufferCreateInfo& createInfo, const void* pInitialData = nullptr);
        [[nodiscard]] vk::Buffer CreateBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, const void* pInitialData = nullptr);
        [[nodiscard]] vk::Buffer CreateIndexBuffer(vk::DeviceSize size, const void* pInitialData = nullptr)   { return CreateBuffer(vk::BufferUsageFlagBits::eIndexBuffer, size, pInitialData); }
        [[nodiscard]] vk::Buffer CreateVertexBuffer(vk::DeviceSize size, const void* pInitialData = nullptr)  { return CreateBuffer(vk::BufferUsageFlagBits::eVertexBuffer, size, pInitialData); }
        [[nodiscard]] vk::Buffer CreateUniformBuffer(vk::DeviceSize size, const void* pInitialData = nullptr) { return CreateBuffer(vk::BufferUsageFlagBits::eUniformBuffer, size, pInitialData); }
        void DestroyBuffer(vk::Buffer buffer);
        [[nodiscard]] vk::DeviceMemory GetBufferMemoryHandle(vk::Buffer buffer) const;
        
        //-----------------------------------------------------------------------------------------------------------
        // IMAGE RESOURCES
        //
        // Images are very similar to buffers in that they are blobs or arrays of memory.
        //	A major difference is that an image knows what type of "pixel" data it contains, so it can be stored
        //	and processed more efficiently by the GPU. Images can also have 1 to 3 dimensions and employ features
        //	such as mipmaps. Because Vulkan separates memory allocation from resource creation, any image created
        //	with CreateImage should be destroyed with DestroyImage so the associated memory is freed.
        //
        // Unlike buffers, images generally are not used directly but instead through ImageView objects that
        //	specify how to interpret the underlying image data. So some helpers also provide facilities to
        //	simultaneously create an ImageView object in addition to the Image.
        //-----------------------------------------------------------------------------------------------------------
        [[nodiscard]] vk::Image CreateImage(const vk::ImageCreateInfo& imageInfo, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);
        [[nodiscard]] std::tuple<vk::Image, vk::ImageView> CreateImageAndView(const vk::ImageCreateInfo& imageInfo, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);
        [[nodiscard]] std::tuple<vk::Image, vk::ImageView> CreateTexture2DImageAndView(vk::Extent2D extents, vk::Format format, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);
        [[nodiscard]] std::tuple<vk::Image, vk::ImageView> CreateCubemapImageAndView(vk::Extent2D extents, vk::Format format, const void* pInitialData = nullptr, vk::DeviceSize initialDataSize = 0);
        void DestroyImage(vk::Image image);
        void DestroyImageAndView(vk::Image image, vk::ImageView view);
        [[nodiscard]] vk::DeviceMemory GetImageMemoryHandle(vk::Image image) const;
        bool UploadImageData(vk::Image image, vk::Extent3D extents, const void* pData, vk::DeviceSize dataSize, uint32_t layerCount = 1);

        //----------------------------------------------------------------------------------------------------
        // GRAPHICS PIPELINES
        //
        // Pipelines are objects that encode the entire state of the execution pipeline on the GPU.
        //	This includes the shaders used, the format of vertex inputs, the format of any uniforms (extra resources),
        // the rasterization settings, output formats, and more.
        //	This makes it harder to change part of the pipeline state dynamically, but improves performance
        //	significantly, which is why all major APIs have adopted this kind of architecture.
        //
        //	In Vulkan there are two objects that are often used together: vk::Pipeline and vk::PipelineLayout.
        //	The Pipeline object contains all the state information about the pipeline, and it even uses PipelineLayout.
        //	The PipelineLayout object only contains information about non-vertex resources needed by the shaders. This
        //	usually means the uniforms exposed to shaders. Not all Pipelines need PipelineLayout objects, especially
        //	if they aren't taking in resources like uniforms.
        //	The GraphicsPipeline object we create here wraps them both for ease of use.
        //
        //	One of the more annoying aspects of creating a Pipeline is that it needs to know the RenderPass
        //	that it will be used within. When drawing to the backbuffer/swapchain (which is what a lot of our drawing
        //	operations target) this means we have to recreate the pipeline whenever the swapchain's RenderPass
        //	changes. Which could be due to switching resolutions or going fullscreen. To reduce the amount of code
        //	needed to manage that, the CreatePipeline helper will return a shared object and the wrapper holds a
        //	weak list of pipelines that it will automatically swap out the RenderPass and recreate the Pipeline
        //	as needed. This is also why it stores a copy of the configuration object.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] std::shared_ptr<GraphicsPipeline> CreatePipeline(const GraphicsPipelineConfig& config);
        [[nodiscard]] std::tuple<vk::Pipeline, vk::PipelineLayout> CreatePipelineAndLayout(const GraphicsPipelineConfig& config);
        void DestroyPipeline(std::shared_ptr<GraphicsPipeline>& pipeline);
        
        //----------------------------------------------------------------------------------------------------
        // SHADER UNIFORMS
        //
        // Uniforms are values that remain uniform across the entire duration of a draw call or similar execution on the GPU.
        //	Contrast these with the per-vertex attributes and per-instance attributes that are provided as vertex buffers.
        //	In Vulkan, these are called DescriptorSets. And the format (or type) of a DescriptorSet is a DescriptorSetLayout.
        //	The term Set is used here because there are often multiple resources that are grouped together and used in shaders.
        //	Though for most of our examples we will be creating DescriptorSets with only one Resource Descriptor in the set.
        //	
        //	And that's why the term Descriptor is used, these objects do not contain the data themselves but merely a reference
        //	to it. To compare it to C++, a DescriptorSetLayout is like a `class` or `struct` declaration -- what the members are.
        //	And consider each member being a pointer or reference. But until we instantiate it, there is no actual data.
        //	A DescriptorSet is an instantiation of a DescriptorSetLayout. And to associate resources (like buffers or images)
        //	with a DescriptorSet we write into it, similar to assigning a value to a pointer member.
        //
        //	Technically a Shader Uniform (also called a Shader Constant) is only one of a handful of additional resources
        //	that can be provided to a shader through the DescriptorSet architecture.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] ShaderUniform CreateUniformForBuffer(int binding, vk::Buffer buffer, vk::DeviceSize size, vk::DeviceSize offset = 0, vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics);
        [[nodiscard]] ShaderUniform CreateUniformForImage(int binding, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAllGraphics);
        void DestroyUniform(ShaderUniform uniform);

        //----------------------------------------------------------------------------------------------------
        // RENDER PASSES
        //
        // In a conventional sense, a Render Pass is when a series of draw calls are issued on a set of outputs.
        //	Multi-pass rendering usually involves drawing the same objects multiple times but with different outputs and pipeline settings.
        //	Often, the output of one pass is fed into another pass to perform certain rendering techniques.
        //
        //	The RenderPass object, when it comes to Vulkan, is just information about the output from a pass.
        // These helpers will create some of the common output formats.
        //	When it comes to ImageLayouts, these are usage instructions to the GPU. There are different ways to store (or access)
        //	image data depending on what operation is being performed. Because performance is key, this is something we now need to explicitly specify.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] vk::RenderPass CreateColorOnlyRenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);
        [[nodiscard]] vk::RenderPass CreateColorAndDepthRenderPass(vk::Format colorFormat, vk::Format depthFormat, vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);

        //----------------------------------------------------------------------------------------------------
        // RENDER TARGETS
        // 
        // A render target (also referred to as a render texture, or offscreen render) is a collection of
        //	Vulkan resources set up to enable rendering to a framebuffer that is not one of the screen buffers.
        //	The backing image that is used in this rendering is often used in other rendering operations.
        //	For example, rendering a scene to a texture and then mapping that texture on an object in the world
        //	such as a TV or similar. But this is also useful for certain techniques that require rendering the
        //	world from a certain perspective, such as rendering a shadow map from the perspective of a light source,
        //	where the map is later used in the main render pass.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] RenderTarget CreateRenderTarget(vk::Extent2D extents, vk::Format colorFormat = vk::Format::eUndefined, vk::Format depthFormat = vk::Format::eUndefined);
        void DestroyRenderTarget(RenderTarget renderTarget);

        //-----------------------------------------------------------------------------------------------------------
        // UTILITIES
        //-----------------------------------------------------------------------------------------------------------
        [[nodiscard]] int FindMemoryTypeIndex(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags);
        [[nodiscard]] vk::DeviceMemory AllocateMemory(vk::MemoryRequirements req, vk::MemoryPropertyFlags flags);
        
        //-----------------------------------------------------------------------------------------------------------
        // GETTERS
        //-----------------------------------------------------------------------------------------------------------
        [[nodiscard]] vk::Instance        GetInstance()                 const { return m_vkbInstance.instance; }
        [[nodiscard]] vk::Device          GetDevice()                   const { return m_vkbDevice.device; }
        [[nodiscard]] vk::PhysicalDevice  GetPhysicalDevice()           const { return m_vkbPhysicalDevice.physical_device; }
        [[nodiscard]] vkb::Swapchain      GetSwapchain()                const { return m_vkbSwapchain; }
        [[nodiscard]] vk::SurfaceKHR      GetDisplaySurface()           const { return m_displaySurface; }
        [[nodiscard]] vk::Viewport        GetDisplayViewport()          const { return m_displayViewport; }
        [[nodiscard]] vk::RenderPass      GetDisplayRenderPass()        const { return m_displayRenderPass; }
        [[nodiscard]] vk::Queue           GetGraphicsQueue()            const { return m_graphicsQueue; }
        [[nodiscard]] uint32_t            GetGraphicsQueueIndex()       const { return m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value(); }
        [[nodiscard]] vk::CommandPool     GetGraphicsCommandPool()      const { return m_graphicsCommandPool; }
        [[nodiscard]] vk::Queue           GetPresentQueue()             const { return m_presentQueue; }
        [[nodiscard]] vk::DescriptorPool  GetDescriptorPool()           const { return m_descriptorPool; }
        [[nodiscard]] vk::PipelineCache   GetPipelineCache()            const { return m_pipelineCache; }
        [[nodiscard]] uint32_t            GetImageCount()               const { return static_cast<uint32_t>(m_framebuffers.size()); }
        [[nodiscard]] uint32_t            GetCurrentFramebufferIndex()  const { return m_currentFramebufferIndex; }

    private:
        bool RebuildSwapchain();
        static VkBool32 DebugLogCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
    };
}

#endif

