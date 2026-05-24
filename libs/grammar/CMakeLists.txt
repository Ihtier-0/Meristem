set(_headers
  include/grammar/Rule.h
  include/grammar/LSystemGrammar.h
)
set(_sources
  src/LSystemGrammar.cpp
)

add_library(grammar STATIC ${_headers} ${_sources})
target_include_directories(grammar PUBLIC include)
target_link_libraries(grammar PUBLIC core)
target_link_libraries(grammar PRIVATE doctest::doctest spdlog::spdlog)
target_compile_definitions(grammar PRIVATE DOCTEST_CONFIG_DISABLE)
set_target_properties(grammar PROPERTIES FOLDER "Meristem")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_headers} ${_sources})

# ── Test sources for the unified test runner in apps/tests ────────────────────
# Recompiles _sources without DOCTEST_CONFIG_DISABLE so TEST_CASE blocks are active.
add_library(grammar_test_sources INTERFACE)
target_sources(grammar_test_sources INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/LSystemGrammar.cpp)
target_include_directories(grammar_test_sources INTERFACE include)
target_link_libraries(grammar_test_sources INTERFACE core doctest::doctest)
