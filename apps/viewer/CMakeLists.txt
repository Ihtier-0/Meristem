add_executable(viewer
  main.cpp
  MainWindow.h    MainWindow.cpp
  TreeCanvas.h    TreeCanvas.cpp
  ControlPanel.h  ControlPanel.cpp
  examples.h
)

set_target_properties(viewer PROPERTIES
  WIN32_EXECUTABLE TRUE
  FOLDER "Meristem"
)

target_link_libraries(viewer PRIVATE
  algorithm
  geometry
  renderer
  glad_gl_core
  Qt6::Core
  Qt6::Widgets
  Qt6::OpenGLWidgets
)

# Deploy Qt DLLs next to the executable after each build (Windows only).
# Pattern from nitroshare-desktop (most widely copied Qt CMake deploy snippet).
if(WIN32)
  get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin "${_qmake}" DIRECTORY)

  find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin}")
  if(NOT WINDEPLOYQT)
    message(FATAL_ERROR "windeployqt not found in ${_qt_bin}")
  endif()

  # Wrap with "env PATH=..." so windeployqt can resolve its own DLL dependencies.
  add_custom_command(TARGET viewer POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_qt_bin}"
            "${WINDEPLOYQT}"
            --verbose 0
            --no-translations
            --no-compiler-runtime
            "$<TARGET_FILE:viewer>"
    COMMENT "windeployqt: copying Qt runtime"
  )
endif()
