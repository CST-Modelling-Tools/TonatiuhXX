# Define the target library
add_library(SunPathLib SHARED "")

# Add source files to the target
target_sources(SunPathLib PRIVATE
    calculators/Location.cpp
    calculators/MB/SunPos.cpp
    calculators/MB/SunPos2.cpp
    calculators/NREL/spa.cpp
    calculators/SunCalculator.cpp
    calculators/SunCalculatorET.cpp
    calculators/SunCalculatorMB.cpp
    calculators/SunCalculatorNREL.cpp
    calculators/SunCalculatorRG.cpp
    data/FormatTMY.cpp
    data/FormatWSN.cpp
    data/SkyModel.cpp
    data/SkyModelPI.cpp
    data/SunFunctor.cpp
    math/geometry/Interval.cpp
    math/geometry/vec2d.cpp
    math/geometry/vec3d.cpp
    math/math.cpp
    math/matrices/MatrixNR.cpp
    math/sampling/ErrorAnalysis.cpp
    math/sampling/Interpolator.cpp
    math/sampling/Summator.cpp
    samplers/SkySampler.cpp
    samplers/SunSpatial.cpp
    samplers/SunTemporal.cpp
    samplers/TimeSampler.cpp
)

# Include directories for this target
target_include_directories(SunPathLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions
target_compile_definitions(SunPathLib PRIVATE
    SUNPATHLIB_EXPORT
)

# Install the library
install(TARGETS SunPathLib
    LIBRARY DESTINATION ${INSTALL_PREFIX}/bin
)

# Install headers
set(SunPathLib_HEADERS
    SunPathLib.h
    calculators/Location.h
    calculators/MB/SunPos.h
    calculators/NREL/spa.h
    calculators/RG/Sun_position_algorithms.h
    calculators/SunCalculator.h
    calculators/SunCalculatorET.h
    calculators/SunCalculatorMB.h
    calculators/SunCalculatorNREL.h
    calculators/SunCalculatorRG.h
    data/FormatTMY.h
    data/FormatWSN.h
    data/SkyModel.h
    data/SkyModelPI.h
    data/SunFunctor.h
    math/geometry/Interval.h
    math/geometry/vec2d.h
    math/geometry/vec3d.h
    math/math.h
    math/matrices/Matrix.h
    math/matrices/MatrixNR.h
    math/sampling/ErrorAnalysis.h
    math/sampling/Interpolator.h
    math/sampling/Summator.h
    samplers/SkySampler.h
    samplers/SunSpatial.h
    samplers/SunTemporal.h
    samplers/TimeSampler.h
)

foreach(header ${SunPathLib_HEADERS})
    get_filename_component(header_path ${header} DIRECTORY)
    install(FILES ${header} DESTINATION ${INSTALL_PREFIX}/include/SunPathLib/${header_path})
endforeach()
