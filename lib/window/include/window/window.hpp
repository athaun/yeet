#pragma once

#include <chrono>
#include <thread>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>
#include <webgpu/wgpu.h>

#include <core/core.hpp>

namespace yeet::window {

namespace WindowManager {

auto poll_every(const usize delay_millis) -> void {
    glfwPollEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_millis));
}

auto init() -> void {
    if ( !static_cast<bool>(glfwInit()) ) {
        eprintln("[FATAL] Failed to initialize GLFW");
        std::abort();
    }
}

auto deinit() -> void {
    glfwTerminate();
}

auto hint() -> void {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

} // namespace WindowManager

struct Window {
    using Self = Window;

    GLFWwindow* handle;
    i32 width;
    i32 height;

    static auto init(
        const Tuple<i32, i32> dimensions,
        const char* name = "yeet"
    ) -> Self {
        WindowManager::hint();
        const auto [width, height] = dimensions;
        auto* window = glfwCreateWindow(width, height, name, nullptr, nullptr);
        if ( window == nullptr ) {
            WindowManager::deinit();
            eprintln("[FATAL] Failed to create GLFW window");
            std::abort();
        }
        return Self{ window, width, height };
    }

    auto deinit() -> void {
        glfwDestroyWindow(self.handle);
        self.handle = nullptr;
    }

    [[nodiscard]] auto should_close() const -> bool {
        return static_cast<bool>(glfwWindowShouldClose(self.handle));
    }

    [[nodiscard]] auto get_wgpu_surface(const wgpu::Instance instance) const
        -> wgpu::Surface {
        return glfwGetWGPUSurface(instance, self.handle);
    }
};

} // namespace yeet::window
