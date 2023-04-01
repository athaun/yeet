#define WEBGPU_CPP_IMPLEMENTATION // THIS IS VERY IMPORTANT
#include <webgpu/webgpu.hpp>
#include <window/window.hpp>

auto main() -> i32 {
  yeet::WindowManager::init();
  auto window = yeet::Window::init("Hello", 800, 600);

  auto _ = defer([&] {
    window.deinit();
    yeet::WindowManager::deinit();
  });

  auto instance = wgpu::createInstance(wgpu::InstanceDescriptor{});
  auto surface = glfwGetWGPUSurface(instance, window.handle);

  println("[INFO] Created WebGPU instance {}", fmt::ptr(&instance));

  auto adapter_options = wgpu::RequestAdapterOptions();
  adapter_options.powerPreference = wgpu::PowerPreference::HighPerformance;

  auto adapter = instance.requestAdapter(adapter_options);

  println("[INFO] Created WebGPU adapter {}", fmt::ptr(&adapter));

  auto device_descriptor = wgpu::DeviceDescriptor();
  device_descriptor.label = "Render Device";

  auto device = adapter.requestDevice(device_descriptor);

  println("[INFO] Created WebGPU device {}", fmt::ptr(&device));

  while (!window.should_close()) {
    yeet::Window::poll_every(50);
  }
}
