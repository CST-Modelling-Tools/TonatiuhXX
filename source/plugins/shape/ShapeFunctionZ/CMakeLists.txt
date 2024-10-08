cmake_minimum_required(VERSION 3.28)
project(ShapeFunctionZ)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Assuming the necessary Qt and Coin3D libraries are found in the top-level CMakeLists.txt

# Include directories, assuming relative paths will be inherited from top-level
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Header files
set(HEADERS
    BVH.h
    ShapeFunctionZ.h
    Triangle.h
)

# Source files
set(SOURCES 
    BVH.cpp
    ShapeFunctionZ.cpp
    Triangle.cpp
)

# Resource files, if any
set(RESOURCES resources.qrc)

# Add the plugin as a library
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES} ${RESOURCES})

# Link libraries using globally defined variables and target links
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${COIN3D_LIBRARY}
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Qml
    TonatiuhLibraries
    TonatiuhKernel
)

# Find the required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets Qml REQUIRED)
