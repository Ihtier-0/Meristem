set(_headers
  include/environment/IEnvironment.h
)
add_library(environment INTERFACE ${_headers})
target_include_directories(environment INTERFACE include)
target_link_libraries(environment INTERFACE core)
set_target_properties(environment PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers})
