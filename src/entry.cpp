#include <vector>
#include <webgpu/webgpu.hpp>
#include <window/window.hpp>

#include <setup/setup.hpp>

auto main() -> i32 {
  yeet::WindowManager::init();
  auto window = yeet::Window::init("Hello", 800, 600);
  auto desc = WGPUInstanceDescriptor{.nextInChain = nullptr};
  auto _ = defer([&] {
    window.deinit();
    yeet::WindowManager::deinit();
  });

  auto instance = wgpuCreateInstance(&desc);

  if (!instance) {
    eprintln("[FATAL] Failed to create WebGPU instance");
    std::abort();
  }

  println("[INFO] Created WebGPU instance {}", fmt::ptr(instance));

  auto surface = glfwGetWGPUSurface(instance, window.handle);
  auto adapter_options = WGPURequestAdapterOptions{
      .nextInChain = nullptr,
      .compatibleSurface = surface,
      .powerPreference = WGPUPowerPreference_HighPerformance,
      .forceFallbackAdapter = false,
  };
  auto adapter = yeet::setup::request_adapter(instance, &adapter_options);
  println("[INFO] Created WebGPU adapter {}", fmt::ptr(adapter));

  auto device_descriptor = WGPUDeviceDescriptor{
      .nextInChain = nullptr,
      .label = "Render Device",
      .requiredFeaturesCount = 0,
      .requiredFeatures = nullptr,
      .requiredLimits = nullptr,
      .defaultQueue =
          {
              .nextInChain = nullptr,
              .label = "Render Queue",
          },
  };
  auto device = yeet::setup::request_device(adapter, &device_descriptor);
  println("[INFO] Created WebGPU device {}", fmt::ptr(device));

  while (!window.should_close()) {
    yeet::Window::poll_every(50);
  }
}
