#pragma once

#include <cstddef>
#include <cstdint>

namespace yeetgpu {

enum class AdapterType : uint32_t {
    Discrete = 0x00000000,
    Integrated = 0x00000001,
    CPU = 0x00000002,
    Unknown = 0x00000003,
    Force32 = 0x7FFFFFFF
};

enum class GPUAddressMode : uint32_t {
    Repeat = 0x00000000,
    MirrorRepeat = 0x00000001,

};

} // namespace yeetgpu
