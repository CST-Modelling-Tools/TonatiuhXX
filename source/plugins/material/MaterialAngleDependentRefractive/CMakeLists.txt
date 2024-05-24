cmake_minimum_required(VERSION 3.28)
project(MaterialAngleDependentRefractive)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    $ENV{TONATIUH_ROOT}/plugins
    $ENV{TONATIUH_ROOT}/src
)

# Header files
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContainerEditorMFVec3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialAngleDependentRefractive.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialAngleDependentRefractiveFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MFVec3.h
)

# Source files
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContainerEditorMFVec3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialAngleDependentRefractive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialAngleDependentRefractiveFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MFVec3.cpp
)

# UI files
set(FORMS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/containerviewermfvec3.ui
)

# Resources
set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialAngleDependentRefractive.qrc
)

# Find the required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

# Enable AUTOMOC, AUTOUIC, and AUTORCC for Qt handling
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define the library or plugin
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES} ${FORMS} ${RESOURCES})

# Link with Qt libraries and potentially other necessary libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Set properties and output directories based on the build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}_debug"
        LIBRARY_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/debug/plugins/MaterialAngleDependentRefractive"
        RUNTIME_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/debug/plugins/MaterialAngleDependentRefractive"
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}d"
        LIBRARY_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/release/plugins/MaterialAngleDependentRefractive"
        RUNTIME_OUTPUT_DIRECTORY "${TONATIUH_ROOT}/bin/release/plugins/MaterialAngleDependentRefractive"
    )
endif()
