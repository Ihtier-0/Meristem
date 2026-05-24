add_library(core INTERFACE
  include/core/types.h
)
target_include_directories(core INTERFACE include)
target_link_libraries(core INTERFACE spdlog::spdlog)
set_target_properties(core PROPERTIES FOLDER "Meristem")
