#pragma once

#include <core/fmt.hpp>
#include <webgpu/webgpu.hpp>

namespace yeet::setup {

[[nodiscard]] auto request_adapter(WGPUInstance instance,
                                   WGPURequestAdapterOptions *options)
    -> WGPUAdapter;

[[nodiscard]] auto request_device(WGPUAdapter adapter,
                                  WGPUDeviceDescriptor const *desc)
    -> WGPUDevice;

} // namespace yeet::setup
