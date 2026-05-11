add_library(grammar STATIC
  include/grammar/Rule.h
  include/grammar/LSystemGrammar.h
  src/LSystemGrammar.cpp
)
target_include_directories(grammar PUBLIC include)
target_link_libraries(grammar PUBLIC core)
