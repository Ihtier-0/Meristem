include(cmake/get_cpm.cmake)

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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
