#include <window/window.hpp>

namespace window = yeet::window;
namespace WindowManager = window::WindowManager;
using Window = window::Window;

constexpr static auto DIMENSIONS = Tuple<i32, i32>{ 640, 480 };

static const auto SHADER = R"(
@vertex
fn vs_main(@location(0) in_vertex_position: vec2<f32>) -> @builtin(position) vec4<f32> {
	return vec4<f32>(in_vertex_position, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.0, 0.4, 1.0, 1.0);
}
)";

static const auto VERTS = Vec<f32>{ -0.5,   -0.5, +0.5,   -0.5, +0.0,   +0.5,
                                    -0.55f, -0.5, -0.05f, +0.5, -0.55f, +0.5 };

static const auto VERTS_COUNT = static_cast<i32>(VERTS.size() / 2);

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
    wgpu::PipelineLayout pipeline_layout;
    wgpu::Buffer vertex_buffer;

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

    // Init Device
    // TODO: (Carter) Find the best defaults cause shit goes wrong real quick
    auto required_limits = wgpu::RequiredLimits{};
    required_limits.limits.maxVertexBufferArrayStride = 2 * 4;   // NOLINT
    required_limits.limits.maxBufferSize = 1024;                 // NOLINT
    required_limits.limits.minStorageBufferOffsetAlignment = 32; // NOLINT
    required_limits.limits.minUniformBufferOffsetAlignment = 32; // NOLINT
    required_limits.limits.maxVertexAttributes = 1; // Change in future
    required_limits.limits.maxVertexBuffers = 1;    // Change in future

    auto device_desc = wgpu::DeviceDescriptor{};
    device_desc.label = "Render Device";
    device_desc.requiredFeaturesCount = 0;
    device_desc.requiredLimits = &required_limits;
    device_desc.defaultQueue.label = "The default queue";

    auto device = adapter.requestDevice(device_desc);
    println("[INFO] Created Device: <{}>", static_cast<void*>(device));

    auto supported_limits = wgpu::SupportedLimits{};
    adapter.getLimits(&supported_limits);
    println(
        "[INFO] Supported Adapter Max Vertex Attributes: <{}>",
        supported_limits.limits.maxVertexAttributes
    );

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

    auto vertex_attrib = wgpu::VertexAttribute{};
    vertex_attrib.shaderLocation = 0;
    vertex_attrib.format = wgpu::VertexFormat::Float32x2;
    vertex_attrib.offset = 0;

    auto vertex_buffer_layout = wgpu::VertexBufferLayout{};
    vertex_buffer_layout.attributeCount = 1;
    vertex_buffer_layout.attributes = &vertex_attrib;
    vertex_buffer_layout.arrayStride = 2 * sizeof(f32);
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

    // Init Vertex Buffer
    auto buffer_desc = wgpu::BufferDescriptor{};
    buffer_desc.size = VERTS.size() * sizeof(f32);
    buffer_desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    buffer_desc.mappedAtCreation = false;
    auto vertex_buffer = device.createBuffer(buffer_desc);

    println(
        "[INFO] Created Vertex Buffer: <{}>",
        static_cast<void*>(vertex_buffer)
    );

    queue.writeBuffer(vertex_buffer, 0, VERTS.data(), buffer_desc.size);

    return { window,   instance,        surface,      adapter,
             device,   queue,           swap_chain,   shader_module,
             pipeline, pipeline_layout, vertex_buffer };
}

auto State::deinit() -> void {
    window.deinit();
    device.destroy(); // This will free all resources created by the device

    free(instance);
    free(surface);
    free(swap_chain);
    free(pipeline);
    free(pipeline_layout);
    free(vertex_buffer);
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
    color_attachment.clearValue = { 0.9f, 0.1f, 0.2f, 1.0f }; // NOLINT

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
