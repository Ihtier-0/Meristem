set(_headers
  include/algorithm/IPlantAlgorithm.h
  include/algorithm/D0LSystemAlgorithm.h
  include/algorithm/StochasticLSystemAlgorithm.h
  include/algorithm/ContextSensitiveLSystemAlgorithm.h
  include/algorithm/ParametricLSystemAlgorithm.h
)
add_library(algorithm INTERFACE ${_headers})
target_include_directories(algorithm INTERFACE include)
target_link_libraries(algorithm INTERFACE grammar environment)
set_target_properties(algorithm PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers})
