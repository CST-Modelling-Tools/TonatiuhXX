#pragma once

#include "SunPath/math/geometry/vec3d.h"

#include <QString>

namespace sp {

class SunPath SkyModel
{
public:
    virtual SkyModel* copy() const;
    virtual ~SkyModel() {}


    // Direct Normal Irradiance
    virtual double findDNI(double alpha) const;

    // Diffuse Horizontal Irradiance
    virtual double findDHI(double alpha) const;

    // Global Horizontal Irradiance
    virtual double findGHI(double alpha) const;


    double findDNIv(const vec3d& v) const;

    virtual QString info() {return "";}
};

} // namespace sp
