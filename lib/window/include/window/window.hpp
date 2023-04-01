#pragma once

#include <chrono>
#include <thread>

#include <glfw3webgpu.h>

#include <core/core.hpp>

namespace yeet {

struct WindowManager {
  using Self = WindowManager;

  static auto init() -> void {
    if (!glfwInit()) {
      eprintln("[FATAL] Failed to initialize GLFW");
      std::abort();
    }
  }

  static auto deinit() -> void { glfwTerminate(); }

  static auto hint() -> void { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); };
};

struct Window {
  using Self = Window;

  GLFWwindow *handle;

  static auto poll_every(const usize delay_millis) -> void {
    glfwPollEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_millis));
  }

  static auto init(const char *name, const f32 width, const f32 height)
      -> Self {
    WindowManager::hint();
    auto window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    if (!window) {
      WindowManager::deinit();
      eprintln("[FATAL] Failed to create GLFW window");
      std::abort();
    }
    return Self{window};
  }

  auto deinit() -> void { glfwDestroyWindow(self.handle); }

  auto should_close() const -> bool {
    return glfwWindowShouldClose(self.handle);
  }
};

} // namespace yeet
