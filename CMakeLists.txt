cmake_minimum_required(VERSION 3.16)

project(Spacecraft_Telemetry_Vizualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Find VTK
# If VTK is not in a standard location, set VTK_DIR before running cmake:
# cmake -DVTK_DIR=/path/to/VTK/build ..
set(VTK_DIR "/home/andrzej/VTK/VTK-9.5.2-install/lib/cmake/vtk-9.5")
find_package(VTK REQUIRED)


# Optional: Print VTK version for verification
message(STATUS "VTK version: ${VTK_VERSION}")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Spacecraft_Telemetry_Vizualizer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(Spacecraft_Telemetry_Vizualizer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Spacecraft_Telemetry_Vizualizer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Link Qt libraries
target_link_libraries(Spacecraft_Telemetry_Vizualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Link VTK libraries
target_link_libraries(Spacecraft_Telemetry_Vizualizer PRIVATE ${VTK_LIBRARIES})

# VTK module initialization (required for VTK 9.0+)
vtk_module_autoinit(
    TARGETS Spacecraft_Telemetry_Vizualizer
    MODULES ${VTK_LIBRARIES}
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Spacecraft_Telemetry_Vizualizer)
endif()
set_target_properties(Spacecraft_Telemetry_Vizualizer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Spacecraft_Telemetry_Vizualizer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Spacecraft_Telemetry_Vizualizer)
endif()
