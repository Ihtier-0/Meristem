add_library(geometry STATIC
  include/geometry/Mesh.h
  include/geometry/IGeometryBuilder.h
  include/geometry/TurtleBuilder2D.h
  src/TurtleBuilder2D.cpp
)
target_include_directories(geometry PUBLIC include)
target_link_libraries(geometry PUBLIC structure)
