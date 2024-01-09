cmake_minimum_required(VERSION 3.28)
project(PhotonsFile)

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
    PhotonsFile.h
    PhotonsFileWidget.h
)

# Source files
set(SOURCES 
    PhotonsFile.cpp
    PhotonsFileWidget.cpp
)

# UI files
# Insert your UI files list here
set(FORMS
    PhotonsFileWidget.ui
)

# Resource files, if any
set(RESOURCES resources.qrc)

# Add the plugin as a library
add_library(PhotonsFile SHARED ${HEADERS} ${SOURCES} ${RESOURCES})

set(COIN3D_LIBRARY /usr/local/lib/libCoin.dylib)
set(SOQT_LIBRARY /usr/local/lib/libSoQt.dylib)

# Link libraries
target_link_libraries(PhotonsFile PRIVATE ${COIN3D_LIBRARY})
target_link_libraries(PhotonsFile PRIVATE ${SOQT_LIBRARY})
target_link_libraries(PhotonsFile PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_link_libraries(PhotonsFile PRIVATE TonatiuhLibraries)
target_link_libraries(PhotonsFile PRIVATE TonatiuhKernel)
