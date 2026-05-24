set(_headers
  include/grammar/Rule.h
  include/grammar/LSystemGrammar.h
)
set(_sources
  src/LSystemGrammar.cpp
)
add_library(grammar STATIC ${_headers} ${_sources})
target_include_directories(grammar PUBLIC include)
target_link_libraries(grammar PUBLIC core)
set_target_properties(grammar PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers} ${_sources})
