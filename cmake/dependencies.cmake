include(cmake/get_cpm.cmake)

# glad2 — OpenGL loader (requires Python 3 at configure time)
CPMAddPackage(
  NAME glad
  GITHUB_REPOSITORY Dav1dde/glad
  GIT_TAG v2.0.6
  SOURCE_SUBDIR cmake
)
glad_add_library(glad_gl_core REPRODUCIBLE LOADER API gl:core=4.6)
foreach(_glad_target glad_gl_core glad-generate-files)
  if(TARGET ${_glad_target})
    set_target_properties(${_glad_target} PROPERTIES FOLDER "glad")
  endif()
endforeach()

# spdlog — logging
CPMAddPackage("gh:gabime/spdlog#v1.14.1")
foreach(_spdlog_target spdlog spdlog_header_only)
  if(TARGET ${_spdlog_target})
    set_target_properties(${_spdlog_target} PROPERTIES FOLDER "spdlog")
  endif()
endforeach()

# doctest — header-only unit test framework
# DOWNLOAD_ONLY: skip doctest's own CMakeLists (uses cmake_minimum_required < 3.5,
# which CMake 3.30+ no longer accepts). We only need the single header.
CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY doctest/doctest
  GIT_TAG v2.4.11
  DOWNLOAD_ONLY YES
)
if(doctest_ADDED)
  add_library(doctest::doctest INTERFACE IMPORTED GLOBAL)
  target_include_directories(doctest::doctest INTERFACE "${doctest_SOURCE_DIR}")
endif()

# Qt6 — UI only
# On Windows: auto-detect the newest msvc2022_64 build in common Qt install roots.
# Override by setting CMAKE_PREFIX_PATH or Qt6_DIR if Qt is somewhere unusual.
if(WIN32)
  find_package(Qt6 QUIET COMPONENTS Core)
  if(NOT Qt6_FOUND)
    file(GLOB _qt6_candidates
      "C:/Qt/6*/msvc2022_64"
      "D:/Qt/6*/msvc2022_64"
      "$ENV{USERPROFILE}/Qt/6*/msvc2022_64"
      "$ENV{LOCALAPPDATA}/Qt/6*/msvc2022_64"
    )
    if(_qt6_candidates)
      list(SORT _qt6_candidates ORDER DESCENDING)   # newest version first
      list(GET _qt6_candidates 0 _qt6_path)
      list(APPEND CMAKE_PREFIX_PATH "${_qt6_path}")
      message(STATUS "Meristem: auto-detected Qt6 at ${_qt6_path}")
    endif()
  endif()
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets Svg)
