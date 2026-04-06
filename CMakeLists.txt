cmake_minimum_required(VERSION 3.20)
project(Spacecraft_Telemetry_Vizualizer VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install prefix" FORCE)
#endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "/home/andrzej/Qt/6.7.2/gcc_64")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

message(STATUS "Qt version: ${Qt6_VERSION}")

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION
    app/src/main.cpp
    gui/src/MainWindow.cpp
    gui/src/MainWindow.h
    gui/src/MainWindow.ui
    rendering/src/SceneManager.cpp
    rendering/src/SceneManager.h
    rendering/src/GLViewport.cpp
    rendering/src/GLViewport.h
    rendering/src/Geometry.cpp
    rendering/src/Geometry.h
    rendering/src/OrbitCamera.cpp
    rendering/src/OrbitCamera.h
    rendering/src/ObjLoader.cpp
    rendering/src/ObjLoader.h
    icons/icons.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    gui/src
    rendering/src
    core/src
    math/src
    third_party/tinyobjloader
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models" "${CMAKE_BINARY_DIR}/models"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/textures" "${CMAKE_BINARY_DIR}/textures"
)

qt_finalize_executable(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY models/ DESTINATION bin/models)
install(DIRECTORY textures/ DESTINATION bin/textures)
