


#include <memory>
#include <string>
#include <vector>

#include "Common/GL/GLInterfaceBase.h"
#include "Common/GL/GLUtil.h"
#include "Common/MsgHandler.h"

#include "VideoBackends/OGL/BoundingBox.h"
#include "VideoBackends/OGL/PerfQuery.h"
#include "VideoBackends/OGL/ProgramShaderCache.h"
#include "VideoBackends/OGL/Render.h"
#include "VideoBackends/OGL/SamplerCache.h"
#include "VideoBackends/OGL/TextureCache.h"
#include "VideoBackends/OGL/TextureConverter.h"
#include "VideoBackends/OGL/VertexManager.h"
#include "VideoBackends/OGL/VideoBackend.h"

#include "VideoCommon/OnScreenDisplay.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

namespace OGL
{
std::string VideoBackend::GetName() const
{
  return "OGL";
}

std::string VideoBackend::GetDisplayName() const
{
  if (GLInterface != nullptr && GLInterface->GetMode() == GLInterfaceMode::MODE_OPENGLES3)
    return "OpenGLES";
  else
    return "OpenGL";
}

void VideoBackend::InitBackendInfo()
{
  g_Config.backend_info.api_type = APIType::OpenGL;
  g_Config.backend_info.MaxTextureSize = 16384;
  g_Config.backend_info.bSupportsExclusiveFullscreen = false;
  g_Config.backend_info.bSupportsOversizedViewports = true;
  g_Config.backend_info.bSupportsGeometryShaders = true;
  g_Config.backend_info.bSupportsComputeShaders = false;
  g_Config.backend_info.bSupports3DVision = false;
  g_Config.backend_info.bSupportsPostProcessing = true;
  g_Config.backend_info.bSupportsSSAA = true;
  g_Config.backend_info.bSupportsReversedDepthRange = true;
  g_Config.backend_info.bSupportsMultithreading = false;
  g_Config.backend_info.bSupportsCopyToVram = true;

        g_Config.backend_info.bSupportsGPUTextureDecoding = true;

    g_Config.backend_info.bSupportsDualSourceBlend = true;
  g_Config.backend_info.bSupportsPrimitiveRestart = true;
  g_Config.backend_info.bSupportsPaletteConversion = true;
  g_Config.backend_info.bSupportsClipControl = true;
  g_Config.backend_info.bSupportsDepthClamp = true;
  g_Config.backend_info.bSupportsST3CTextures = false;
  g_Config.backend_info.bSupportsBPTCTextures = false;

  g_Config.backend_info.Adapters.clear();

    g_Config.backend_info.AAModes = {1, 2, 4, 8};
}

bool VideoBackend::InitializeGLExtensions()
{
    if (!GLExtensions::Init())
  {
            PanicAlert("GPU: OGL ERROR: Does your video card support OpenGL 2.0?");
    return false;
  }

  if (GLExtensions::Version() < 300)
  {
        PanicAlert("GPU: OGL ERROR: Need OpenGL version 3.\n"
               "GPU: Does your video card support OpenGL 3?");
    return false;
  }

  return true;
}

bool VideoBackend::FillBackendInfo()
{
    GLint numvertexattribs = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numvertexattribs);
  if (numvertexattribs < 16)
  {
    PanicAlert("GPU: OGL ERROR: Number of attributes %d not enough.\n"
               "GPU: Does your video card support OpenGL 2.x?",
               numvertexattribs);
    return false;
  }

    GLint max_texture_size = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
  g_Config.backend_info.MaxTextureSize = static_cast<u32>(max_texture_size);
  if (max_texture_size < 1024)
  {
    PanicAlert("GL_MAX_TEXTURE_SIZE too small at %i - must be at least 1024.", max_texture_size);
    return false;
  }

    return true;
}

bool VideoBackend::Initialize(void* window_handle)
{
  InitBackendInfo();
  InitializeShared();

  GLUtil::InitInterface();
  GLInterface->SetMode(GLInterfaceMode::MODE_DETECT);
  if (!GLInterface->Create(window_handle, g_ActiveConfig.stereo_mode == StereoMode::QuadBuffer))
    return false;

  GLInterface->MakeCurrent();
  if (!InitializeGLExtensions() || !FillBackendInfo())
    return false;

  g_renderer = std::make_unique<Renderer>();
  g_vertex_manager = std::make_unique<VertexManager>();
  g_perf_query = GetPerfQuery();
  ProgramShaderCache::Init();
  g_texture_cache = std::make_unique<TextureCache>();
  g_sampler_cache = std::make_unique<SamplerCache>();
  g_shader_cache = std::make_unique<VideoCommon::ShaderCache>();
  static_cast<Renderer*>(g_renderer.get())->Init();
  TextureConverter::Init();
  BoundingBox::Init(g_renderer->GetTargetWidth(), g_renderer->GetTargetHeight());
  return g_shader_cache->Initialize();
}

void VideoBackend::Shutdown()
{
  g_shader_cache->Shutdown();
  g_renderer->Shutdown();
  BoundingBox::Shutdown();
  TextureConverter::Shutdown();
  g_shader_cache.reset();
  g_sampler_cache.reset();
  g_texture_cache.reset();
  ProgramShaderCache::Shutdown();
  g_perf_query.reset();
  g_vertex_manager.reset();
  g_renderer.reset();
  GLInterface->ClearCurrent();
  GLInterface->Shutdown();
  GLInterface.reset();
  ShutdownShared();
}
}
