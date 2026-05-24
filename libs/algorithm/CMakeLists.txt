add_library(algorithm INTERFACE
  include/algorithm/IPlantAlgorithm.h
  include/algorithm/D0LSystemAlgorithm.h
  include/algorithm/StochasticLSystemAlgorithm.h
  include/algorithm/ContextSensitiveLSystemAlgorithm.h
  include/algorithm/ParametricLSystemAlgorithm.h
)
target_include_directories(algorithm INTERFACE include)
target_link_libraries(algorithm INTERFACE grammar structure environment)
set_target_properties(algorithm PROPERTIES FOLDER "Meristem")
