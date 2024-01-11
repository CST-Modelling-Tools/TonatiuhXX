#include "SkySampler.h"

#include "SunPath/samplers/SunSpatial.h"

// simplified trapezoidal mesh for |latitudes| < 45 degrees

namespace sp {


double SkyNode::kernel(const vec3d& r) const
{
    double arg = dot(v, r) - 1.;
    arg *= coeff;
    if (arg < -20.) return 0.;
    return exp(arg);
}


SkySampler::SkySampler(SunSpatial* sunSpatial):
    m_sunSpatial(sunSpatial)
{

}

// rho: resolution
// sigma: half-width
bool SkySampler::sample(double rho, double sigma, bool symEW)
{
    if (rho < 0.) return false;
    if (sigma == 0.) sigma = 3.*rho;

    QVector<Equatorial> points;
    QVector<double> deltas = findDeclinations().sampleResolution(rho);
    for (double delta : deltas) {
        QVector<double> omegas;
        if (symEW) {
            Interval in = findHours(delta);
            int divs = 2*round(in.size()/(2*rho));
            omegas = in.sampleDivisions(divs);
        } else
            omegas = findHours(delta).sampleResolution(rho);
        for (double omega : omegas)
            points << Equatorial(omega, delta);
    }

    QVector<SkyNode> nodes;
    SunCalculator* sc = m_sunSpatial->calculator();
    for (Equatorial& p : points)
        nodes << SkyNode(sc->findVector(p), sigma);

    m_sunSpatial->setSkyNodes(nodes);

    QString description = QString("Resolution: %1 deg").arg(rho/degree, 0, 'f', 4);
    m_sunSpatial->setInfo(description);
    return true;
}

// declinations above horizon
Interval SkySampler::findDeclinations()
{
    SunCalculator* sc = m_sunSpatial->calculator();
    double eps = sc->obliquity();
    double phi = sc->location().latitude();

    return Interval(
        std::max(-eps, phi - pi/2.),
        std::min(eps, phi + pi/2.)
    );
}

// hour angles above horizon for a given declination
Interval SkySampler::findHours(double delta)
{
    SunCalculator* sc = m_sunSpatial->calculator();
    double tanPhi = tan(sc->location().latitude());
    double q = -tanPhi*tan(delta);

    double temp;
    if (std::abs(q) < 1.)
        temp = acos(q);
    else
        temp = -std::copysign(1., q)*pi;

    return Interval(-temp, temp);
}


} // namespace sp
