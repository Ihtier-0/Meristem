add_library(environment INTERFACE
  include/environment/IEnvironment.h
)
target_include_directories(environment INTERFACE include)
target_link_libraries(environment INTERFACE core)
set_target_properties(environment PROPERTIES FOLDER "Meristem")
