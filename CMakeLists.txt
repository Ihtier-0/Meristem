cmake_minimum_required(VERSION 3.30)
project(Meristem VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/dependencies.cmake)
include(cmake/platform.cmake)
include(cmake/version.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

enable_testing()

# Build all test executables and run them via ctest.
# Each lib's CMakeLists adds its test exe:  add_dependencies(all_tests <target>)
add_custom_target(all_tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running all tests"
)
set_target_properties(all_tests PROPERTIES FOLDER "Tests")

add_subdirectory(libs)
add_subdirectory(apps)
