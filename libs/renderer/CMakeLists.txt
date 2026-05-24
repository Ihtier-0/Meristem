set(_headers
  include/renderer/DrawCall.h
  include/renderer/IRenderer.h
  include/renderer/OpenGLRenderer.h
)
set(_sources
  src/OpenGLRenderer.cpp
)
add_library(renderer STATIC ${_headers} ${_sources})
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC geometry glad_gl_core)
set_target_properties(renderer PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers} ${_sources})
