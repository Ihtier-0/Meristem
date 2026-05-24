add_executable(tests main.cpp)

target_link_libraries(tests PRIVATE
  grammar_test_sources
  # future: algorithm_test_sources, geometry_test_sources, ...
)

set_target_properties(tests PROPERTIES FOLDER "Tests" AUTOMOC OFF)
add_test(NAME tests COMMAND tests)
add_dependencies(all_tests tests)
