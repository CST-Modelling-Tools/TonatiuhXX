#include "SkyModelPI.h"

namespace sp {

SkyModelPI::SkyModelPI()
{
    m_i0 = 1618.;

    m_taub = 0.606;
    m_b = 0.491;

    m_taud = 2.698;
    m_d = 0.187;

    m_taug = 0.464;
    m_g = 0.402;
}

SkyModelPI* SkyModelPI::copy() const
{
    return new SkyModelPI(*this);
}

double SkyModelPI::findDNI(double alpha) const
{
    if (alpha < 0.) return 0.;
    alpha = clamp(alpha, 0., 90.*degree);
    double temp = pow(sin(alpha), m_b);
    double ans = m_i0*exp(-m_taub/temp);
    return ans;
}

double SkyModelPI::findDHI(double alpha) const
{
    if (alpha < 0.) return 0.;
    alpha = clamp(alpha, 0., 90.*degree);
    double temp = pow(sin(alpha), m_d);
    double ans = m_i0*exp(-m_taud/temp);
    return ans;
}

double SkyModelPI::findGHI(double alpha) const
{
    if (alpha < 0.) return 0.;
    alpha = clamp(alpha, 0., 90.*degree);
    double sina = sin(alpha);
    double temp = pow(sina, m_g);
    double ans = m_i0*exp(-m_taug/temp)*sina;
    return ans;
}

} // namespace sp
