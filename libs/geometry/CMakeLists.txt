set(_headers
  include/geometry/Mesh.h
  include/geometry/IGeometryBuilder.h
  include/geometry/TurtleBuilder2D.h
)
set(_sources
  src/TurtleBuilder2D.cpp
)
add_library(geometry STATIC ${_headers} ${_sources})
target_include_directories(geometry PUBLIC include)
target_link_libraries(geometry PUBLIC core)
set_target_properties(geometry PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers} ${_sources})
