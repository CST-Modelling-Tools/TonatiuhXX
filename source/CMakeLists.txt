cmake_minimum_required(VERSION 3.28)
project(TonatiuhXX VERSION 0.1.8.16)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Configure compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
    # GCC-specific configurations for MacOS
    add_compile_definitions(COIN_NOT_DLL SOQT_NOT_DLL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-attributes")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# MacOS may use different paths for libraries, adjust as necessary
link_directories(/usr/local/lib)

# Find the Qt6 package
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

# Global include paths
include_directories(${PROJECT_SOURCE_DIR}/source)

# Include Coin3D headers
include_directories(/usr/local/include)
include_directories(/usr/local/include/Inventor)

# Include Eigen headers
include_directories(/usr/local/include/eigen3/)

# Qt6 specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set the path to the desired Qt version
set(Qt6_DIR "/opt/homebrew/Cellar/qt6/6.1.1/lib/cmake/Qt6")

# Find the required Qt components
find_package(Qt6 COMPONENTS PrintSupport REQUIRED)

# Add subdirectories
add_subdirectory(libraries)
add_subdirectory(kernel)
add_subdirectory(application)
