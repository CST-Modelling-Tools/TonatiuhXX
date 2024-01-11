#include "SkyModel.h"

namespace sp {

SkyModel* SkyModel::copy() const
{
    return new SkyModel(*this);
}

double SkyModel::findDNI(double alpha) const
{
    Q_UNUSED(alpha)
    return 0.;
}

double SkyModel::findDHI(double alpha) const
{
    Q_UNUSED(alpha)
    return 0.;
}

double SkyModel::findGHI(double alpha) const
{
    return findDNI(alpha)*sin(alpha) + findDHI(alpha);
}

double SkyModel::findDNIv(const vec3d& v) const
{
    double alpha = asin(v.z);
    return findDNI(alpha);
}

} // namespace sp
