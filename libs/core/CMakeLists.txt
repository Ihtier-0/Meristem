set(_headers
  include/core/math.h
  include/core/types.h
)
set(_generated
  "${PROJECT_BINARY_DIR}/include/core/version.h"
)
add_library(core INTERFACE ${_headers} ${_generated})
target_include_directories(core INTERFACE
  include
  "${PROJECT_BINARY_DIR}/include"
)
target_link_libraries(core INTERFACE spdlog::spdlog)
set_target_properties(core PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers})
source_group(TREE "${PROJECT_BINARY_DIR}/include"
  PREFIX "generated"
  FILES ${_generated}
)
