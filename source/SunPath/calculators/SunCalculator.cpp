#include "SunPath/calculators/SunCalculator.h"

using namespace sp;


double  SunCalculator::s_obliquity = 23.4354*degree;


SunCalculator::SunCalculator()
{
    setLocation(Location());
}

SunCalculator* SunCalculator::copy() const
{
    return new SunCalculator(*this);
}

void SunCalculator::setLocation(const Location& location)
{
    m_location = location;
    m_cosPhi = cos(m_location.latitude());
    m_sinPhi = sin(m_location.latitude());
}

vec3d SunCalculator::findVector(const Horizontal& hc) const
{
    double gamma = hc.azimuth();
    double alpha = hc.elevation();

    double cosAlpha = cos(alpha);

    return vec3d(
        sin(gamma)*cosAlpha,
        cos(gamma)*cosAlpha,
        sin(alpha)
    );
}

vec3d SunCalculator::findVector(const Equatorial& ec) const
{
    double omega = ec.hour();
    double delta = ec.declination();

    double cosOmega = cos(omega);
    double sinOmega = sin(omega);
    double cosDelta = cos(delta);
    double sinDelta = sin(delta);
    double cosOmegaDelta = cosOmega*cosDelta;

    return vec3d(
        -sinOmega*cosDelta,
        sinDelta*m_cosPhi - cosOmegaDelta*m_sinPhi,
        sinDelta*m_sinPhi + cosOmegaDelta*m_cosPhi
                );
}

Horizontal SunCalculator::findHorizontal(const vec3d& v) const
{
    double alpha = asin(v.z);
    if (eqz(alpha)) alpha = 0.;
    double gamma = atan2(v.x, v.y);
    return Horizontal(gamma, alpha);
}

Horizontal SunCalculator::findHorizontalV(const QDateTime& /*t*/) const
{
    return Horizontal(0., 0.);
}

Equatorial SunCalculator::findEquatorial(const vec3d& v) const
{
    double omega = atan2(-v.x, -v.y*m_sinPhi + v.z*m_cosPhi);
    double delta = asin(v.y*m_cosPhi + v.z*m_sinPhi);
    return Equatorial(omega, delta);
}

QDateTime SunCalculator::findTime(const Horizontal& /*hc*/, const QDateTime& t0) const
{
    return t0;
}
