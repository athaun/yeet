#include "webgpu/webgpu.h"
#include <setup/setup.hpp>

namespace yeet::setup {

[[nodiscard]] auto request_adapter(WGPUInstance instance,
                                   WGPURequestAdapterOptions *options)
    -> WGPUAdapter {
  struct Context {
    WGPUAdapter adapter = nullptr;
    bool request_ended = false;
  };

  auto context = Context{};
  auto on_request_adapter_complete = [](WGPURequestAdapterStatus status,
                                        WGPUAdapter adapter,
                                        char const *message, void *pContext) {
    auto &context = *reinterpret_cast<Context *>(pContext);

    if (status == WGPURequestAdapterStatus_Success) {
      context.adapter = adapter;
    } else {
      eprintln("[FATAL] Failed to create WebGPU adapter: {}", message);
      std::abort();
    }
    context.request_ended = true;
  };

  wgpuInstanceRequestAdapter(instance, options, on_request_adapter_complete,
                             (void *)&context);

  if (!context.request_ended) {
    eprintln("[FATAL] Failed to create WebGPU adapter");
    std::abort();
  }

  return context.adapter;
}

[[nodiscard]] auto request_device(WGPUAdapter adapter,
                                  WGPUDeviceDescriptor const *desc)
    -> WGPUDevice {
  struct Context {
    WGPUDevice device = nullptr;
    bool request_ended = false;
  };

  auto context = Context{};
  auto on_request_device_complete = [](WGPURequestDeviceStatus status,
                                       WGPUDevice device, char const *message,
                                       void *p_context) {
    auto &context = *reinterpret_cast<Context *>(p_context);
    if (status == WGPURequestDeviceStatus_Success) {
      context.device = device;
    } else {
      eprintln("[FATAL] Failed to create WebGPU device: {}", message);
      std::abort();
    }
    context.request_ended = true;
  };

  wgpuAdapterRequestDevice(adapter, desc, on_request_device_complete,
                           (void *)&context);

  if (!context.request_ended) {
    eprintln("[FATAL] Failed to create WebGPU device");
    std::abort();
  }

  auto device = context.device;
  auto callback = [](WGPUErrorType type, char const *message,
                     [[maybe_unused]] void *context) {
    eprintln("[FATAL] WebGPU device error: {}", message);
  };

  wgpuDeviceSetUncapturedErrorCallback(device, callback, nullptr);

  return device;
}

} // namespace yeet::setup
