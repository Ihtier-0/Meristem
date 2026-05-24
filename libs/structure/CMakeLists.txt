add_library(structure INTERFACE
  include/structure/PlantStructure.h
  include/structure/StringStructure.h
  include/structure/TreeGraph.h
)
target_include_directories(structure INTERFACE include)
target_link_libraries(structure INTERFACE core)
set_target_properties(structure PROPERTIES FOLDER "Meristem")
