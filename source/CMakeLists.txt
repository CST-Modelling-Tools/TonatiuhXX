cmake_minimum_required(VERSION 3.28)
set(ProjectName TonatiuhXX)

project(${ProjectName} VERSION 0.1.8.16)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set default to Release build type if none specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(APPLE)
    # Select the Qt6 version we want to use
    set(Qt6_DIR "/opt/homebrew/Cellar/qt6/6.1.1/lib/cmake/Qt6")
    
    # Include Coin3D and Eigen headers (specific to macOS)
    include_directories(/usr/local/include)
    include_directories(/usr/local/include/Inventor)
    include_directories(/usr/local/include/eigen3/)

    # Link directories for macOS
    link_directories(/usr/local/lib)    
elseif(WIN32)
    # Set the Qt6 path for Windows if needed
    # set(Qt6_DIR "path/to/qt6/on/windows")
elseif(UNIX AND NOT APPLE) # Linux
    # Set the Qt6 path for Linux if needed
    # set(Qt6_DIR "path/to/qt6/on/linux")
endif()

# Find the Qt6 package
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

# Qt6 specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Global compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wno-deprecated-declarations -Wno-attributes)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    endif()
    # Add definitions for Coin3D on MacOS
    if(APPLE)
        add_compile_definitions(COIN_NOT_DLL SOQT_NOT_DLL)
    endif()
endif()

# Speficying the path of the Coin3D and SoQt libraries
if(APPLE)
    set(COIN3D_LIBRARY /usr/local/lib/libCoin.dylib)
    set(SOQT_LIBRARY /usr/local/lib/libSoQt.dylib)
elseif(WIN32)
    # Set paths for Windows
elseif(UNIX AND NOT APPLE)
    # Set paths for Linux
endif()

# Add subdirectories
add_subdirectory(libraries)
add_subdirectory(kernel)
add_subdirectory(application)
add_subdirectory(plugins)