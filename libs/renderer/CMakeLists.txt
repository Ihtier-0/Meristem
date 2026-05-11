add_library(renderer STATIC
  include/renderer/DrawCall.h
  include/renderer/IRenderer.h
  include/renderer/OpenGLRenderer.h
  src/OpenGLRenderer.cpp
)
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC geometry glad_gl_core)
