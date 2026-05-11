add_executable(viewer main.cpp)

target_link_libraries(viewer PRIVATE
  algorithm
  geometry
  renderer
  glad_gl_core
  glfw
  imgui
)
