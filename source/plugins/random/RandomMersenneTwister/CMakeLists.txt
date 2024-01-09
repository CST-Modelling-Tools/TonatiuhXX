cmake_minimum_required(VERSION 3.28)
project(RandomMersenneTwister)

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
    RandomMersenneTwister.h
)

# Source files
set(SOURCES 
    RandomMersenneTwister.cpp
)

# Add the plugin as a library
add_library(RandomMersenneTwister SHARED ${HEADERS} ${SOURCES} ${RESOURCES})

set(COIN3D_LIBRARY /usr/local/lib/libCoin.dylib)
set(SOQT_LIBRARY /usr/local/lib/libSoQt.dylib)

# Link libraries
target_link_libraries(RandomMersenneTwister PRIVATE ${COIN3D_LIBRARY})
target_link_libraries(RandomMersenneTwister PRIVATE ${SOQT_LIBRARY})
target_link_libraries(RandomMersenneTwister PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_link_libraries(RandomMersenneTwister PRIVATE TonatiuhLibraries)
target_link_libraries(RandomMersenneTwister PRIVATE TonatiuhKernel)

