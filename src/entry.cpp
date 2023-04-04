#include <window/window.hpp>

namespace window = yeet::window;
namespace WindowManager = window::WindowManager;
using Window = window::Window;

template<>
struct fmt::formatter<WGPULimits> {
    constexpr static auto parse(format_parse_context& ctx)
        -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const wgpu::Limits& limits, FormatContext& ctx)
        -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "wgpu::Limits:\n"
            "  maxTextureDimension1D: {},\n"
            "  maxTextureDimension2d: {},\n"
            "  maxTextureDimension3d: {},\n"
            "  maxTextureArrayLayers: {},\n"
            "  maxBindGroups: {},\n"
            "  maxBindingsPerBindGroup: {},\n"
            "  maxDynamicUniformBuffersPerPipelineLayout: {},\n"
            "  maxDynamicStorageBuffersPerPipelineLayout: {},\n"
            "  maxSampledTexturesPerShaderStage: {},\n"
            "  maxUniformBuffersPerShaderStage: {},\n"
            "  maxUniformBufferBindingSize: {},\n"
            "  maxStorageBufferBindingSize: {},\n"
            "  minUniformBufferOffsetAlignment: {},\n"
            "  minStorageBufferOffsetAlignment: {},\n"
            "  maxVertexBuffers: {},\n"
            "  maxBufferSize: {},\n"
            "  maxVertexAttributes: {},\n"
            "  maxVertexBufferArrayStride: {},\n"
            "  maxInterStageShaderComponents: {},\n"
            "  maxInterStageShaderVariables: {},\n"
            "  maxColorAttachments: {},\n"
            "  maxColorAttachmentBytesPerSample: {},\n"
            "  maxComputeWorkgroupStorageSize: {},\n"
            "  maxComputeInvocationsPerWorkgroup: {},\n"
            "  maxComputeWorkgroupSizeX: {},\n"
            "  maxComputeWorkgroupSizeY: {},\n"
            "  maxComputeWorkgroupSizeZ: {},\n"
            "  maxComputeWorkgroupsPerDimension: {}\n",
            limits.maxTextureDimension1D,
            limits.maxTextureDimension2D,
            limits.maxTextureDimension3D,
            limits.maxTextureArrayLayers,
            limits.maxBindGroups,
            limits.maxBindingsPerBindGroup,
            limits.maxDynamicUniformBuffersPerPipelineLayout,
            limits.maxDynamicStorageBuffersPerPipelineLayout,
            limits.maxSampledTexturesPerShaderStage,
            limits.maxUniformBuffersPerShaderStage,
            limits.maxUniformBufferBindingSize,
            limits.maxStorageBufferBindingSize,
            limits.minUniformBufferOffsetAlignment,
            limits.minStorageBufferOffsetAlignment,
            limits.maxVertexBuffers,
            limits.maxBufferSize,
            limits.maxVertexAttributes,
            limits.maxVertexBufferArrayStride,
            limits.maxInterStageShaderComponents,
            limits.maxInterStageShaderVariables,
            limits.maxColorAttachments,
            limits.maxColorAttachmentBytesPerSample,
            limits.maxComputeWorkgroupStorageSize,
            limits.maxComputeInvocationsPerWorkgroup,
            limits.maxComputeWorkgroupSizeX,
            limits.maxComputeWorkgroupSizeY,
            limits.maxComputeWorkgroupSizeZ,
            limits.maxComputeWorkgroupsPerDimension
        );
    }
};

constexpr static auto DIMENSIONS = Tuple<i32, i32>{ 640, 480 };

static const auto SHADER = R"(
/**
 * A structure with fields labeled with vertex attribute locations can be used
 * as input to the entry point of a shader.
 */
struct VertexInput {
	@location(0) position: vec2<f32>,
	@location(1) color: vec3<f32>,
};
/**
 * A structure with fields labeled with builtins and locations can also be used
 * as *output* of the vertex shader, which is also the input of the fragment
 * shader.
 */
struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	// The location here does not refer to a vertex attribute, it just means
	// that this field must be handled by the rasterizer.
	// (It can also refer to another field of another struct that would be used
	// as input to the fragment shader.)
	@location(0) color: vec3<f32>,
};
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.position = vec4<f32>(in.position, 0.0, 1.0);
	out.color = in.color; // forward to the fragment shader
	return out;
}
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
	return vec4<f32>(in.color, 1.0);
}
)";

constexpr static auto VERTS =
    Arr<f32, 30>{ -0.5, -0.5, 1.0, 0.0, 0.0, +0.5, -0.5, 0.0, 1.0, 0.0,
                  +0.5, +0.5, 0.0, 0.0, 1.0, -0.5, +0.5, 1.0, 1.0, 0.0 };
constexpr static auto VERTS_COUNT = static_cast<i32>(VERTS.size() / 5);

constexpr static auto INDICES = Arr<u16, 6>{
    0, 1, 2, 0, 2, 3,
};
constexpr static auto INDICES_COUNT = static_cast<i32>(INDICES.size());

struct State {
    Window window;
    wgpu::Instance instance;
    wgpu::Surface surface;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::SwapChain swap_chain;
    wgpu::ShaderModule shader_module;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertex_buffer;
    wgpu::Buffer index_buffer;

    static auto init(Tuple<i32, i32> dimensions) -> State;
    auto deinit() -> void;

    auto render() -> void;
};

auto State::init(const Tuple<i32, i32> dimensions) -> State {
    // Init Window
    auto window = Window::init(dimensions);

    // Init Instance
    auto instance_desc = wgpu::InstanceDescriptor{};
    auto instance = wgpu::createInstance(instance_desc);
    if ( instance == nullptr ) {
        eprintln("[FATAL] Failed to create WebGPU instance");
        std::abort();
    }
    println("[INFO] Created Instance: <{}>", static_cast<void*>(instance));

    // Init Surface
    auto surface = window.get_wgpu_surface(instance);
    println("[INFO] Created Surface: <{}>", static_cast<void*>(surface));

    // Init Adapter
    auto adapter_opts = wgpu::RequestAdapterOptions{};
    adapter_opts.compatibleSurface = surface;
    auto adapter = instance.requestAdapter(adapter_opts);
    println("[INFO] Created Adapter: <{}>", static_cast<void*>(adapter));

    auto supported_limits = wgpu::SupportedLimits{};
    adapter.getLimits(&supported_limits);
    println("[INFO] Supported Adapter Limits: \n{}", supported_limits.limits);

    // Init Device
    auto required_limits = wgpu::RequiredLimits{};
    required_limits.limits = supported_limits.limits;
    required_limits.limits.maxVertexAttributes = 2;
    required_limits.limits.maxVertexBuffers = 1;

    auto device_desc = wgpu::DeviceDescriptor{};
    device_desc.label = "Render Device";
    device_desc.requiredFeaturesCount = 0;
    device_desc.requiredLimits = &required_limits;
    device_desc.defaultQueue.label = "The default queue";

    auto device = adapter.requestDevice(device_desc);
    println("[INFO] Created Device: <{}>", static_cast<void*>(device));

    device.getLimits(&supported_limits);
    println(
        "[INFO] Supported Device Max Vertex Attributes: <{}>",
        supported_limits.limits.maxVertexAttributes
    );

    // Init Queue
    auto queue = device.getQueue();
    println("[INFO] Created Queue: <{}>", static_cast<void*>(queue));

    // Init SwapChain
    auto swap_chain_format = surface.getPreferredFormat(adapter);
    auto swap_chain_desc = wgpu::SwapChainDescriptor{};
    swap_chain_desc.width = std::get<0>(DIMENSIONS);
    swap_chain_desc.height = std::get<1>(DIMENSIONS);
    swap_chain_desc.format = swap_chain_format;
    swap_chain_desc.usage = wgpu::TextureUsage::RenderAttachment;
    swap_chain_desc.presentMode = wgpu::PresentMode::Fifo;
    auto swap_chain = device.createSwapChain(surface, swap_chain_desc);
    println("[INFO] Created SwapChain: <{}>", static_cast<void*>(swap_chain));

    // Init Shader Module
    auto shader_code_desc = wgpu::ShaderModuleWGSLDescriptor{};
    shader_code_desc.chain.next = nullptr;
    shader_code_desc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    shader_code_desc.code = SHADER;

    auto shader_desc = wgpu::ShaderModuleDescriptor{};
    shader_desc.hintCount = 0;
    shader_desc.hints = nullptr;
    shader_desc.nextInChain = &shader_code_desc.chain;

    auto shader_module = device.createShaderModule(shader_desc);
    if ( shader_module == nullptr ) {
        eprintln("[FATAL] Failed to create Shader Module");
        std::abort();
    }
    println(
        "[INFO] Created Shader Module: <{}>",
        static_cast<void*>(shader_module)
    );

    auto primitive_state = wgpu::PrimitiveState{};
    primitive_state.topology = wgpu::PrimitiveTopology::TriangleList;
    primitive_state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    primitive_state.frontFace = wgpu::FrontFace::CCW;
    primitive_state.cullMode = wgpu::CullMode::None;

    auto vertex_attribs = Arr<wgpu::VertexAttribute, 2>{};
    vertex_attribs [0].shaderLocation = 0;
    vertex_attribs [0].format = wgpu::VertexFormat::Float32x2;
    vertex_attribs [0].offset = 0;
    vertex_attribs [1].shaderLocation = 1;
    vertex_attribs [1].format = wgpu::VertexFormat::Float32x3;
    vertex_attribs [1].offset = 2 * sizeof(f32);

    auto vertex_buffer_layout = wgpu::VertexBufferLayout{};
    vertex_buffer_layout.attributeCount =
        static_cast<u32>(vertex_attribs.size());
    vertex_buffer_layout.attributes = vertex_attribs.data();
    vertex_buffer_layout.arrayStride = 5 * sizeof(f32); // NOLINT
    vertex_buffer_layout.stepMode = wgpu::VertexStepMode::Vertex;

    auto blend_state = wgpu::BlendState{};
    blend_state.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blend_state.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blend_state.color.operation = wgpu::BlendOperation::Add;
    blend_state.alpha.srcFactor = wgpu::BlendFactor::Zero;
    blend_state.alpha.dstFactor = wgpu::BlendFactor::One;
    blend_state.alpha.operation = wgpu::BlendOperation::Add;

    auto color_target = wgpu::ColorTargetState{};
    color_target.format = swap_chain_format;
    color_target.blend = &blend_state;
    color_target.writeMask = wgpu::ColorWriteMask::All;

    auto fragment_state = wgpu::FragmentState{};
    fragment_state.module = shader_module;
    fragment_state.entryPoint = "fs_main";
    fragment_state.constantCount = 0;
    fragment_state.constants = nullptr;
    fragment_state.targetCount = 1;
    fragment_state.targets = &color_target;

    auto pipeline_layout_desc = wgpu::PipelineLayoutDescriptor{};
    pipeline_layout_desc.bindGroupLayoutCount = 0;
    pipeline_layout_desc.bindGroupLayouts = nullptr;
    auto pipeline_layout = device.createPipelineLayout(pipeline_layout_desc);

    auto pipeline_desc = wgpu::RenderPipelineDescriptor{};
    pipeline_desc.primitive = primitive_state;
    pipeline_desc.vertex.bufferCount = 1;
    pipeline_desc.vertex.buffers = &vertex_buffer_layout;
    pipeline_desc.vertex.module = shader_module;
    pipeline_desc.vertex.entryPoint = "vs_main";
    pipeline_desc.vertex.constantCount = 0;
    pipeline_desc.vertex.constants = nullptr;
    pipeline_desc.fragment = &fragment_state;
    pipeline_desc.depthStencil = nullptr;
    pipeline_desc.multisample.count = 1;
    pipeline_desc.multisample.mask = ~0U;
    pipeline_desc.multisample.alphaToCoverageEnabled = false;
    pipeline_desc.layout = pipeline_layout;

    auto pipeline = device.createRenderPipeline(pipeline_desc);
    println(
        "[INFO] Created Render Pipeline: <{}>",
        static_cast<void*>(pipeline)
    );
    free(pipeline_layout);

    // Init Vertex Buffer
    auto buffer_desc = wgpu::BufferDescriptor{};
    buffer_desc.size = VERTS.size() * sizeof(f32);
    buffer_desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    buffer_desc.mappedAtCreation = false;
    auto vertex_buffer = device.createBuffer(buffer_desc);
    queue.writeBuffer(vertex_buffer, 0, VERTS.data(), buffer_desc.size);
    println(
        "[INFO] Created Vertex Buffer: <{}>",
        static_cast<void*>(vertex_buffer)
    );

    // Init Index Buffer
    buffer_desc.size = INDICES.size() * sizeof(u16);
    buffer_desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
    auto index_buffer = device.createBuffer(buffer_desc);
    println(
        "[INFO] Created Index Buffer: <{}>",
        static_cast<void*>(index_buffer)
    );

    queue.writeBuffer(index_buffer, 0, INDICES.data(), buffer_desc.size);

    return { window,   instance,      surface,     adapter,
             device,   queue,         swap_chain,  shader_module,
             pipeline, vertex_buffer, index_buffer };
}

auto State::deinit() -> void {
    self.window.deinit();
    self.device.destroy(); // This will free all resources created by the device

    free(self.instance);
    free(self.surface);
    free(self.swap_chain);
    free(self.shader_module);
    free(self.pipeline);
    free(self.vertex_buffer);
    free(self.index_buffer);
}

auto State::render() -> void {
    // Init Next Texture
    auto next_texture =
        static_cast<wgpu::TextureView>(self.swap_chain.getCurrentTextureView());
    if ( next_texture == nullptr ) {
        eprintln("[FATAL] Could not texture from swap chain");
        std::abort();
    }

    // Init Encoder
    auto encoder_desc = wgpu::CommandEncoderDescriptor{};
    encoder_desc.label = "Command Encoder";
    auto encoder = device.createCommandEncoder(encoder_desc);

    // Init Render Pass
    auto color_attachment = wgpu::RenderPassColorAttachment{};
    color_attachment.view = next_texture;
    color_attachment.resolveTarget = nullptr;
    color_attachment.loadOp = wgpu::LoadOp::Clear;
    color_attachment.storeOp = wgpu::StoreOp::Store;
    color_attachment.clearValue = { 0.05f, 0.05f, 0.05f, 1.0f }; // NOLINT

    auto render_pass_desc = wgpu::RenderPassDescriptor{};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachment;
    render_pass_desc.depthStencilAttachment = nullptr;
    render_pass_desc.timestampWriteCount = 0;
    render_pass_desc.timestampWrites = nullptr;

    auto render_pass = encoder.beginRenderPass(render_pass_desc);

    render_pass.setPipeline(self.pipeline);

    render_pass
        .setVertexBuffer(0, self.vertex_buffer, 0, VERTS.size() * sizeof(f32));
    render_pass.draw(VERTS_COUNT, 1, 0, 0);
    render_pass.setIndexBuffer(
        self.index_buffer,
        wgpu::IndexFormat::Uint16,
        0,
        INDICES.size() * sizeof(u16)
    );
    render_pass.drawIndexed(INDICES_COUNT, 1, 0, 0, 0);
    // render_pass.draw(VERTS_COUNT, 1, 0, 0); FOR VERTS ONLY

    render_pass.end();

    wgpuTextureViewDrop(next_texture);

    auto command_buf_desc = wgpu::CommandBufferDescriptor{};
    command_buf_desc.label = "Command Buffer";
    auto command = encoder.finish(command_buf_desc);

    self.queue.submit(command);

    self.swap_chain.present();
}

auto main() -> i32 {
    WindowManager::init();
    auto state = State::init(DIMENSIONS); // NOLINT

    auto state_deinit = defer([&] {
        state.deinit();
        WindowManager::deinit();
    });

    for ( ; !state.window.should_close(); ) {
        WindowManager::poll_every(50); // NOLINT
        state.render();
    }
}
