cmake_minimum_required(VERSION 3.28)
project(ComponentHeliostatField)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the external configuration
include(../../config.cmake)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    $ENV{TONATIUH_ROOT}/plugins
    $ENV{TONATIUH_ROOT}/src/source/gui
    $ENV{TONATIUH_ROOT}/src/source/raytracing
)

# Header files
set(HEADERS
    src/ComponentHeliostatField.h
    src/ComponentHeliostatFieldFactory.h
    src/HeliostatFieldWizard.h
    src/InputTable.h
)

# Source files
set(SOURCES
    src/ComponentHeliostatField.cpp
    src/ComponentHeliostatFieldFactory.cpp
    src/HeliostatFieldWizard.cpp
    src/InputTable.cpp
)

# UI files
set(FORMS
    src/heliostatfieldwizard.ui
    src/inputtablewidget.ui
)

# Resources
set(RESOURCES
    src/ComponentHeliostatField.qrc
)

# Find the required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

# Enable AUTOMOC, AUTOUIC, and AUTORCC for Qt handling
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define the library or plugin
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES} ${FORMS} ${RESOURCES})

# Link with Qt libraries and potentially other necessary libraries (like Coin3D if used elsewhere)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Set properties and output directories based on the build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}_debug"
        LIBRARY_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/debug/plugins/ComponentHeliostatField"
        RUNTIME_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/debug/plugins/ComponentHeliostatField"
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}d"
        LIBRARY_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/release/plugins/ComponentHeliostatField"
        RUNTIME_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/release/plugins/ComponentHeliostatField"
    )
endif()