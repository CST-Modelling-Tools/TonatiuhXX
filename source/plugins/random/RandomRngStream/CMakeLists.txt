cmake_minimum_required(VERSION 3.28)
project(RandomRngStream)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the path to the desired Qt version
set(Qt6_DIR "/opt/homebrew/Cellar/qt6/6.1.1/lib/cmake/Qt6")

# Find the required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Add definitions
add_definitions(-DTONATIUH_KERNEL_EXPORT)

# Include Coin3D headers
include_directories(/usr/local/include)
include_directories(/usr/local/include/Inventor)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libraries)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel)

#Header files
set(HEADERS
    src/RandomRngStream.h
    src/RandomRngStreamFactory.h
)

# Source files
set(SOURCES 
    src/RandomRngStream.cpp
    src/RandomRngStreamFactory.cpp
)

# Define the library
add_library(RandomRngStream SHARED ${HEADERS} ${SOURCES})

set(COIN3D_LIBRARY /usr/local/lib/libCoin.dylib)
set(SOQT_LIBRARY /usr/local/lib/libSoQt.dylib)

# Link libraries
target_link_libraries(PhotonsFile PRIVATE ${COIN3D_LIBRARY})
target_link_libraries(PhotonsFile PRIVATE ${SOQT_LIBRARY})
target_link_libraries(PhotonsFile PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_link_libraries(PhotonsFile PRIVATE TonatiuhLibraries)
target_link_libraries(PhotonsFile PRIVATE TonatiuhKernel)

# Set the output directories for different build types
set_target_properties(RandomRngStream PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${TONATIUH_ROOT}/bin/debug/plugins/RandomRngStream
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${TONATIUH_ROOT}/bin/release/plugins/RandomRngStream
)

# Additional configurations for the target can be set here
