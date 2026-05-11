include(cmake/get_cpm.cmake)

# GLFW — window and input
CPMAddPackage(
  NAME glfw
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.4
  OPTIONS
    "GLFW_BUILD_DOCS OFF"
    "GLFW_BUILD_TESTS OFF"
    "GLFW_BUILD_EXAMPLES OFF"
)

# GLM — math (Vec2/Vec3/Mat4/Quat)
CPMAddPackage("gh:g-truc/glm#1.0.1")

# glad2 — OpenGL loader (requires Python 3 at configure time)
CPMAddPackage(
  NAME glad
  GITHUB_REPOSITORY Dav1dde/glad
  GIT_TAG v2.0.6
  SOURCE_SUBDIR cmake
)
glad_add_library(glad_gl_core REPRODUCIBLE LOADER API gl:core=4.6)

# spdlog — logging
CPMAddPackage("gh:gabime/spdlog#v1.14.1")

# Dear ImGui (header + backends, no built-in CMake target)
CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.91.6
  DOWNLOAD_ONLY YES
)
if(imgui_ADDED)
  file(REMOVE_RECURSE ${imgui_SOURCE_DIR}/examples)
endif()
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)
