cmake_minimum_required(VERSION 3.28)
project(RandomMersenneTwister)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Assuming the necessary Qt and Coin3D libraries are found in the top-level CMakeLists.txt
# This reduces redundancy and ensures consistency across the build system

# Include directories, ensure compatibility with platform specifics
if(APPLE)
    include_directories(
        /usr/local/include
        /usr/local/include/Inventor
    )
elseif(UNIX AND NOT APPLE) # Linux settings
    include_directories(
        /usr/include
        /usr/include/Inventor
    )
endif()

#Header files
set(HEADERS
    RandomMersenneTwister.h
)

# Source files
set(SOURCES 
    RandomMersenneTwister.cpp
)

# Add the plugin as a library
add_library(RandomMersenneTwister SHARED ${HEADERS} ${SOURCES} ${RESOURCES})

# Utilize globally configured libraries and components
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${COIN3D_LIBRARY} 
    ${SOQT_LIBRARY} 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    TonatiuhLibraries
    TonatiuhKernel
)

# Find the required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)