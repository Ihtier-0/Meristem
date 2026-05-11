add_library(core INTERFACE
  include/core/types.h
)
target_include_directories(core INTERFACE include)
target_link_libraries(core INTERFACE glm::glm spdlog::spdlog)
