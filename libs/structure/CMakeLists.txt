set(_headers
  include/structure/StringStructure.h
)
add_library(structure INTERFACE ${_headers})
target_include_directories(structure INTERFACE include)
target_link_libraries(structure INTERFACE core)
set_target_properties(structure PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers})
