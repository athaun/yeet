#pragma once

#include <webgpu/webgpu.h>

namespace yeetgpu {

#define END \
    }       \
    ;

#define WGPU_INNER(Type)          \
    struct Type {                 \
        using Self = Type;        \
        using WSelf = WGPU##Type; \
        WSelf inner;              \
                                  \
      public:

constexpr static auto FORCE32 = std::numeric_limits<uint32_t>::max() / 2;

#define ENUM_WITH_FORCE32(name, ...) \
    enum class name : uint32_t { __VA_ARGS__, Force32 = FORCE32 };

// Enumerations
ENUM_WITH_FORCE32(AdapterType, DiscreteGPU, IntegratedGPU, CPU, Unknown)
ENUM_WITH_FORCE32(AddressMode, Repeat, MirrorRepeat, ClampToEdge)
ENUM_WITH_FORCE32(
    BackendType,
    Null,
    WebGPU,
    D3D11,
    D3D12,
    Metal,
    Vulkan,
    OpenGL,
    OpenGLES
)
ENUM_WITH_FORCE32(
    BlendFactor,
    Zero,
    One,
    Src,
    OneMinusSrc,
    SrcAlpha,
    OneMinusSrcAlpha,
    Dst,
    OneMinusDst,
    DstAlpha,
    OneMinusDstAlpha,
    SrcAlphaSaturated,
    Constant,
    OneMinusConstant
)
ENUM_WITH_FORCE32(BlendOperation, Add, Subtract, ReverseSubtract, Min, Max)
ENUM_WITH_FORCE32(
    BufferBindingType,
    Undefined,
    Uniform,
    Storage,
    ReadOnlyStorage
)
ENUM_WITH_FORCE32(
    BufferMapAsyncStatus,
    Success,
    Error,
    Unknown,
    DeviceLost,
    DestroyedBeforeCallback,
    UnmappedBeforeCallback
)
ENUM_WITH_FORCE32(BufferMapState, Unmapped, Pending, Mapped)
ENUM_WITH_FORCE32(
    CompareFunction,
    Undefined,
    Never,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    Always
)
ENUM_WITH_FORCE32(
    CompilationInfoRequestStatus,
    Success,
    Error,
    DeviceLost,
    Unknown
)
ENUM_WITH_FORCE32(CompilationInfoType, Error, Warning, Info)
ENUM_WITH_FORCE32(ComputePassTimeStampLocation, Beginning, End)
ENUM_WITH_FORCE32(
    CreatePipelineAsyncStatus,
    Success,
    ValidationError,
    InternalError,
    DeviceLost,
    DeviceDestroyed,
    Unknown
)
ENUM_WITH_FORCE32(CullMode, None, Front, Back)
ENUM_WITH_FORCE32(DeviceLostReason, Undefined, Destroyed)
ENUM_WITH_FORCE32(ErrorFilter, Validation, OutOfMemory, Internal)
ENUM_WITH_FORCE32(
    ErrorType,
    NoError,
    Validation,
    OutOfMemory,
    Internal,
    Unknown
)

// FeatureName -- TextureUsageFlags

// Structs
template<typename T>
concept IsWGPUType = std::is_same_v<typename T::Self, typename T::Self> &&
                     std::is_same_v<typename T::WSelf, typename T::WSelf>;

template<typename T>
concept IsWGPUResource = IsWGPUType<T> && requires(T t) {
    { T::init() } -> std::same_as<typename T::WSelf>;
    { t.deinit() } -> std::same_as<void>;
};

struct ChainedStruct {
    const ChainedStruct* next;
    SType s_type;
};

struct ChainedStructOut {
    const ChainedStructOut* next;
    SType s_type;
};

struct AdapterProps {
    ChainedStructOut* next;
    uint32_t vendor_id;
    const char* architecutre;
    uint32_t device_id;
    const char* name;
    const char* driver_description;
    AdapterType adapter_type;
    BackendType backend_type;
};

} // namespace yeetgpu
