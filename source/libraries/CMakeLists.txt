cmake_minimum_required(VERSION 3.28)
project(TonatiuhLibraries)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the path to the desired Qt version
set(Qt6_DIR "/opt/homebrew/Cellar/qt6/6.1.1/lib/cmake/Qt6")

# Find the required Qt components
find_package(Qt6 COMPONENTS PrintSupport REQUIRED)

# Add definitions
add_compile_definitions(TONATIUH_LIBRARIES_EXPORT SUNPATHLIB_EXPORT)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Header files
set(HEADERS 
    TonatiuhLibraries.h 
    auxiliary/tiny_obj_loader.h 
    auxiliary/Trace.h 
    Coin3D/ContainerEditorMFVec2.h 
    Coin3D/ContainerViewerMFVec2.h 
    Coin3D/DoubleValuesDelegate.h 
    Coin3D/FieldEditor.h 
    Coin3D/MFVec2.h 
    Coin3D/UserMField.h 
    Coin3D/UserSField.h 
    DistMesh/PolygonMesh.h 
    DistMesh/constants.h 
    DistMesh/delaunator.h 
    DistMesh/distance_function.h 
    DistMesh/distmesh.h 
    DistMesh/functional.h 
    DistMesh/helper.h 
    DistMesh/triangulation.h 
    DistMesh/utils.h 
    math/1D/Grid.h 
    math/1D/Interval.h 
    math/1D/IntervalPeriodic.h 
    math/2D/Box2D.h 
    math/2D/Interpolation2D.h 
    math/2D/Matrix2D.h
    math/2D/vec2d.h
    math/2D/vec2i.h
    math/3D/Box3D.h
    math/3D/Matrix4x4.h
    math/3D/Ray.h
    math/3D/Transform.h
    math/3D/Transform3D.h
    math/3D/vec3d.h
    math/gcf.h
    QCustomPlot/qcustomplot.h
    sun/sunpos.h
)

# Source files
set(SOURCES 
    auxiliary/Trace.cpp
    auxiliary/tiny_obj_loader.cpp
    Coin3D/ContainerEditorMFVec2.cpp
    Coin3D/ContainerViewerMFVec2.cpp
    Coin3D/DoubleValuesDelegate.cpp
    Coin3D/MFVec2.cpp
    Coin3D/UserMField.cpp
    Coin3D/UserSField.cpp
    DistMesh/PolygonMesh.cpp
    DistMesh/distance_function.cpp
    DistMesh/distmesh.cpp
    DistMesh/functional.cpp
    DistMesh/triangulation.cpp
    DistMesh/utils.cpp
    math/1D/Grid.cpp
    math/1D/Interval.cpp
    math/1D/IntervalPeriodic.cpp
    math/2D/Box2D.cpp
    math/2D/vec2d.cpp
    math/2D/vec2i.cpp
    math/3D/Box3D.cpp
    math/3D/Matrix4x4.cpp
    math/3D/Transform.cpp
    math/3D/Transform3D.cpp
    math/3D/vec3d.cpp
    math/gcf.cpp
    QCustomPlot/qcustomplot.cpp
    sun/sunpos.cpp
)

# UI files
#set(FORMS Coin3D/ContainerViewerMFVec2.ui)

#set(UI_OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated_ui)

#file(MAKE_DIRECTORY ${UI_OUTPUT_DIR})

#qt6_wrap_ui(UI_HEADERS ${FORMS} WORKING_DIRECTORY ${UI_OUTPUT_DIR})

# Create the library
add_library(TonatiuhLibraries SHARED ${SOURCES} ${HEADERS} ${UI_HEADERS})

set(COIN3D_LIBRARY /usr/local/lib/libCoin.dylib)

# Link libraries
target_link_libraries(TonatiuhLibraries PRIVATE ${COIN3D_LIBRARY})
target_link_libraries(TonatiuhLibraries PRIVATE Qt6::PrintSupport)
