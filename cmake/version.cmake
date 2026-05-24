# Single integer: major*10000 + minor*100 + patch  (same scheme as TinyXr)
math(EXPR MERISTEM_VERSION
  "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}")
set(MERISTEM_VERSION_STRING
  "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

configure_file(
  "${PROJECT_SOURCE_DIR}/libs/core/include/core/version.h.in"
  "${PROJECT_BINARY_DIR}/include/core/version.h"
)
