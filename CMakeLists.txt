cmake_minimum_required(VERSION 3.20)
project(Spacecraft_Telemetry_Vizualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "/home/andrzej/Qt/6.10.0/gcc_64")
#set(VTK_DIR "/home/andrzej/VTK/install/lib/cmake/vtk-9.5")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
find_package(VTK REQUIRED)

message(STATUS "VTK version: ${VTK_VERSION}")

qt_add_executable(Spacecraft_Telemetry_Vizualizer MANUAL_FINALIZATION
    app/src/main.cpp
    gui/src/MainWindow.cpp
    gui/src/MainWindow.h
    gui/src/MainWindow.ui
    rendering/src/SceneManager.cpp
    rendering/src/SceneManager.h
    icons/icons.qrc
)

target_include_directories(Spacecraft_Telemetry_Vizualizer PRIVATE
    gui/src
    rendering/src
    core/src
    math/src
)

target_link_libraries(Spacecraft_Telemetry_Vizualizer PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    ${VTK_LIBRARIES}
)

set_target_properties(Spacecraft_Telemetry_Vizualizer PROPERTIES
    BUILD_RPATH "/home/andrzej/Qt/6.10.0/gcc_64/lib"
    INSTALL_RPATH "/home/andrzej/Qt/6.10.0/gcc_64/lib"
)

#vtk_module_autoinit(TARGETS Spacecraft_Telemetry_Vizualizer MODULES ${VTK_LIBRARIES})

add_custom_command(TARGET Spacecraft_Telemetry_Vizualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models" "${CMAKE_BINARY_DIR}/models"
)

qt_finalize_executable(Spacecraft_Telemetry_Vizualizer)
