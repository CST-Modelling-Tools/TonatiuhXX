#include "SunFunctor.h"

namespace sp {


double SunFunctorPanelCos::operator()(const vec3d& s) const
{
    double ns = dot(n, s);
    if (ns <= 0.) return 0.;
    return ns;
}

double SunFunctorHeliostatCos::operator()(const vec3d& s) const
{
    vec3d n = (s + t).normalized();
    double ns = dot(n, s);
    if (ns <= 0.) return 0;
    return ns;
}

double SunFunctorHeliostatFieldOld::operator()(const vec3d& s) const
{
    if (s.z <= 0.) return 0.;

    vec3d n = (s + t).normalized();
    double ns = dot(n, s);
    if (ns <= 0.) return 0.;
    double ans = ns;

    ans *= s.z; // shading
    return ans;
}

double SunFunctorHeliostatField::operator()(const vec3d& s) const
{
    if (s.z <= 0.) return 0.;

    vec3d n = (s + t).normalized();
    double ns = dot(n, s);
    if (ns <= 0.) return 0.;
    double ans = ns*ns;

    double q = s.z/0.5; // shading
    if (q < 1.)
        ans *= 2*q - q*q;

    return ans;
}

double SunFunctorDNI::operator()(const sp::vec3d& s) const
{
    return model.findDNIv(s);
}

double SunFunctorWeighted::operator()(const vec3d& s) const
{
    double w = (*sw)(s);
    if (w >= 0.) w *= (*sf)(s);
    return w;
}


} // namespace sp
