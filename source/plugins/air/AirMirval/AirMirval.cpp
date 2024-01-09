#include "AirMirval.h"

SO_NODE_SOURCE(AirMirval)


void AirMirval::initClass()
{
    SO_NODE_INIT_CLASS(AirMirval, AirTransmission, "Air");
}

AirMirval::AirMirval()
{
    SO_NODE_CONSTRUCTOR(AirMirval);
}

double AirMirval::transmission(double distance) const
{
    double d = distance/1000.;
	double t;
    if (d <= 1.)
        t = 0.99321 - 0.1176*d + 0.0197*d*d;
	else
        t = exp(-0.1106*d);
    return t;
}

// Reference:
// Leary, P.L., Hankins, J.D., 1979. A user's guide for MIRVAL - a computer code for programming designs of heliostat-receiver optics for central receiver solar power plants. Sandia Laboratories Report, SAND77-8280. pp. 14.
