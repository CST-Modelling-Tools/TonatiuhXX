cmake_minimum_required(VERSION 3.28)
project(TonatiuhXX_Plugins)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/..)
include_directories(${PROJECT_SOURCE_DIR}/../libraries)
include_directories(${PROJECT_SOURCE_DIR}/../kernel)

# Assuming Coin3D and Qt6 are already found in the higher-level CMakeLists.txt
# No need to find them again here

# Add each plugin subdirectory
add_subdirectory(air)
# add_subdirectory(component) # Uncomment this if you want to include the 'component' plugin
add_subdirectory(material)
add_subdirectory(photons)
add_subdirectory(random)
add_subdirectory(shape)
# add_subdirectory(sun)
# add_subdirectory(trackers)

