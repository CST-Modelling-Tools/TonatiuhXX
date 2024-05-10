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

# Global compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wno-deprecated-declarations -Wno-attributes)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    endif()
endif()

# Platform-specific configurations
if(APPLE)
    # macOS specific settings
    set(Qt6_DIR "/opt/homebrew/Cellar/qt6/6.1.1/lib/cmake/Qt6")
    include_directories(
      /usr/local/include 
      /usr/local/include/Inventor
      /usr/local/include/eigen3/)
    link_directories(/usr/local/lib)
    set(COIN3D_LIBRARY /usr/local/lib/libCoin.dylib)
    set(SOQT_LIBRARY /usr/local/lib/libSoQt.dylib)
    add_compile_definitions(COIN_NOT_DLL SOQT_NOT_DLL)
elseif(WIN32)
    # Windows specific settings
    # set(Qt6_DIR "path/to/qt6/on/windows")
    # include_directories(paths for Windows)
    # link_directories(paths for Windows)
    # set(COIN3D_LIBRARY path for Windows)
    # set(SOQT_LIBRARY path for Windows)
elseif(UNIX)
    # Linux specific settings
    # set(Qt6_DIR "path/to/qt6/on/linux")
    # include_directories(paths for Linux)
    # link_directories(paths for Linux)
    # set(COIN3D_LIBRARY path for Linux)
    # set(SOQT_LIBRARY path for Linux)
endif()

# Find the Qt6 package
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

# Qt6 specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add subdirectories
add_subdirectory(libraries)
add_subdirectory(kernel)
add_subdirectory(SunPath)
add_subdirectory(application)
add_subdirectory(plugins)