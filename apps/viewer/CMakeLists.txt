add_executable(viewer
  main.cpp
  Window.h    Window.cpp
  ViewerUI.h  ViewerUI.cpp
  examples.h
)

target_link_libraries(viewer PRIVATE
  algorithm
  geometry
  renderer
  glad_gl_core
  glfw
  imgui
)
