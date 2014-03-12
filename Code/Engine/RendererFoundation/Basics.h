#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Basics/Types/Id.h>
#include <Foundation/Basics/Types/RefCounted.h>

// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
  #ifdef BUILDSYSTEM_BUILDING_RENDERERFOUNDATION_LIB
    #define EZ_RENDERERFOUNDATION_DLL __declspec(dllexport)
    #define EZ_RENDERERFOUNDATION_TEMPLATE
  #else
    #define EZ_RENDERERFOUNDATION_DLL __declspec(dllimport)
    #define EZ_RENDERERFOUNDATION_TEMPLATE extern
  #endif
#else
  #define EZ_RENDERERFOUNDATION_DLL
  #define EZ_RENDERERFOUNDATION_TEMPLATE
#endif

// Necessary array sizes
#define EZ_GAL_MAX_CONSTANT_BUFFER_COUNT 16
#define EZ_GAL_MAX_SHADER_RESOURCE_VIEW_COUNT 16
#define EZ_GAL_MAX_VERTEX_BUFFER_COUNT 16
#define EZ_GAL_MAX_RENDERTARGET_COUNT 8

// Forward declarations

struct ezGALDeviceCreationDescription;
struct ezGALSwapChainCreationDescription;
struct ezGALShaderCreationDescription;
struct ezGALTextureCreationDescription;
struct ezGALBufferCreationDescription;
struct ezGALDepthStencilStateCreationDescription;
struct ezGALBlendStateCreationDescription;
struct ezGALRasterizerStateCreationDescription;
struct ezGALRenderTargetConfigCreationDescription;
struct ezGALVertexDeclarationCreationDescription;
struct ezGALQueryCreationDescription;
struct ezGALSamplerStateCreationDescription;
struct ezGALResourceViewCreationDescription;
struct ezGALRenderTargetViewCreationDescription;

class ezGALSwapChain;
class ezGALShader;
class ezGALTexture;
class ezGALBuffer;
class ezGALDepthStencilState;
class ezGALBlendState;
class ezGALRasterizerState;
class ezGALRenderTargetConfig;
class ezGALVertexDeclaration;
class ezGALFence;
class ezGALQuery;
class ezGALSamplerState;
class ezGALResourceView;
class ezGALRenderTargetView;
class ezGALDevice;
class ezGALContext;

// Basic enums
struct ezGALPrimitiveTopology
{
  enum Enum
  {
    Triangles,

    // TODO

    ENUM_COUNT
  };
};

struct ezGALIndexType
{
  enum Enum
  {
    UShort,
    UInt,

    ENUM_COUNT
  };
};


struct ezGALShaderStage
{
  enum Enum
  {
    VertexShader,
    HullShader,
    DomainShader,
    GeometryShader,
    PixelShader,

    ComputeShader,

    ENUM_COUNT
  };

};

struct ezGALMSAASampleCount
{
  enum Enum
  {
    None = 1,
    TwoSamples = 2,
    FourSamples = 4,
    EightSamples = 8,

    ENUM_COUNT = 4
  };
};

struct ezGALTextureType
{
  enum Enum
  {
    Texture2D = 0,
    TextureCube,
    Texture3D,

    ENUM_COUNT
  };
};

struct ezGALBlend
{
  enum Enum
  {
    Zero = 0,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DestAlpha,
    InvDestAlpha,
    DestColor,
    InvDestColor,
    SrcAlphaSaturated,
    BlendFactor,
    InvBlendFactor,

    ENUM_COUNT
  };
};

struct ezGALBlendOp
{
  enum Enum
  {
    Add = 0,
    Subtract,
    RevSubtract,
    Min,
    Max,

    ENUM_COUNT
  };
};

struct ezGALStencilOp
{
  enum Enum
  {
    Keep = 0,
    Zero,
    Replace,
    IncrementSaturated,
    DecrementSaturated,
    Invert,
    Increment,
    Decrement,

    ENUM_COUNT
  };
};

struct ezGALCompareFunc
{
  enum Enum
  {
    Never = 0,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,

    ENUM_COUNT
  };
};

struct ezGALCullMode
{
  enum Enum
  {
    None = 0,
    Front = 1,
    Back = 2,

    ENUM_COUNT
  };
};

struct ezGALTextureFilterMode
{
  enum Enum
  {
    Point = 0,
    Linear,
    Anisotropic,

    ENUM_COUNT
  };
};

struct ezGALTextureAddressMode
{
  enum Enum
  {
    Wrap = 0,
    Mirror,
    Clamp,
    Border,
    MirrorOnce,

    ENUM_COUNT
  };
};

// Basic structs
struct ezGALTextureSubresource
{
  ezUInt32 m_uiMipLevel;
  ezUInt32 m_uiArraySlice;
};

struct ezGALSystemMemoryDescription
{
  EZ_FORCE_INLINE ezGALSystemMemoryDescription()
    : m_pData(NULL),
      m_uiRowPitch(0),
      m_uiSlicePitch(0)
  {
  }


  void* m_pData;
  ezUInt32 m_uiRowPitch;
  ezUInt32 m_uiSlicePitch;
};

/// \brief Base class for GAL objects, stores a creation description of the object and also allows for reference counting.
template<typename CreationDescription> class ezGALObjectBase : public ezRefCounted
{
public:

  EZ_FORCE_INLINE ezGALObjectBase(const CreationDescription& Description)
    : m_Description(Description)
  {
  }

  EZ_FORCE_INLINE const CreationDescription& GetDescription() const
  {
    return m_Description;
  }

protected:
  const CreationDescription m_Description;
};

// Handles
namespace ezGAL
{
  typedef ezGenericId<16, 16> ez16_16Id;
  typedef ezGenericId<18, 14> ez18_14Id;
  typedef ezGenericId<20, 12> ez20_12Id;
}

class ezGALSwapChainHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALSwapChainHandle, ezGAL::ez16_16Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALShaderHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALShaderHandle, ezGAL::ez18_14Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALTextureHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALTextureHandle, ezGAL::ez18_14Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALBufferHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALBufferHandle, ezGAL::ez18_14Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALResourceViewHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALResourceViewHandle, ezGAL::ez18_14Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALRenderTargetViewHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALRenderTargetViewHandle, ezGAL::ez18_14Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALDepthStencilStateHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALDepthStencilStateHandle, ezGAL::ez16_16Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALBlendStateHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALBlendStateHandle, ezGAL::ez16_16Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALRasterizerStateHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALRasterizerStateHandle, ezGAL::ez16_16Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALSamplerStateHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALSamplerStateHandle, ezGAL::ez16_16Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALRenderTargetConfigHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALRenderTargetConfigHandle, ezGAL::ez16_16Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALVertexDeclarationHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALVertexDeclarationHandle, ezGAL::ez18_14Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};

class ezGALFenceHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALFenceHandle, ezGAL::ez20_12Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};


class ezGALQueryHandle
{
  EZ_DECLARE_HANDLE_TYPE(ezGALQueryHandle, ezGAL::ez20_12Id);

  friend class ezGALDevice;
  friend struct ezGALContextState;
  friend class ezGALContext;
};
