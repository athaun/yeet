#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <window/window.hpp>

namespace window = yeet::window;
namespace WindowManager = window::WindowManager;
using Window = window::Window;

auto main() -> i32 {
    WindowManager::init();
    auto window = Window::init({ 640, 480 }); // NOLINT

    auto window_deinit = defer([&] {
        window.deinit();
        WindowManager::deinit();
    });

    for ( ; !window.should_close(); ) {
        WindowManager::poll_every(50); // NOLINT
    }
}
