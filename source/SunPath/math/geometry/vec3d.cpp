#include "vec3d.h"

namespace sp {

const vec3d vec3d::Zero(0., 0., 0.);
const vec3d vec3d::One(1., 1., 1.);
const vec3d vec3d::UnitX(1., 0., 0.);
const vec3d vec3d::UnitY(0., 1., 0.);
const vec3d vec3d::UnitZ(0., 0., 1.);


bool vec3d::operator==(const vec3d& v) const
{
    return equals(x, v.x) &&
           equals(y, v.y) &&
           equals(z, v.z);
}

bool vec3d::operator!=(const vec3d& v) const
{
    return !(*this == v);
}

int vec3d::maxDimension() const
{
    if (x > y && x > z)
        return 0;
    else if (y > z)
        return 1;
    return 2;
}

vec3d vec3d::directionAE(double azimuth, double elevation)// in radians
{
    double cosAlpha = cos(elevation);
    return vec3d(
        cosAlpha*sin(azimuth),
        cosAlpha*cos(azimuth),
        sin(elevation)
    );
}

std::ostream& operator<<(std::ostream& os, const vec3d& v)
{
    os << v.x << ", " << v.y << ", " << v.z;
    return os;
}

} // namespace sp
