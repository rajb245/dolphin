add_library(videoogl
  GPUTimer.h
  OGLBoundingBox.cpp
  OGLBoundingBox.h
  OGLConfig.cpp
  OGLConfig.h
  OGLGfx.cpp
  OGLGfx.h
  OGLMain.cpp
  OGLNativeVertexFormat.cpp
  OGLPerfQuery.cpp
  OGLPerfQuery.h
  OGLPipeline.cpp
  OGLPipeline.h
  OGLShader.cpp
  OGLShader.h
  OGLStreamBuffer.cpp
  OGLStreamBuffer.h
  OGLTexture.cpp
  OGLTexture.h
  OGLVertexManager.cpp
  OGLVertexManager.h
  ProgramShaderCache.cpp
  ProgramShaderCache.h
  SamplerCache.cpp
  SamplerCache.h
  VideoBackend.h
)

target_link_libraries(videoogl PUBLIC common videocommon)
if(ENABLE_X11 AND X11_FOUND AND TARGET PkgConfig::X11)
  target_link_libraries(videoogl PRIVATE PkgConfig::X11)
endif()

if(MSVC)
  # Add precompiled header
  target_link_libraries(videoogl PRIVATE use_pch)
endif()
