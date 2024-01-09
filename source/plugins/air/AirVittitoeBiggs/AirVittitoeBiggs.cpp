#include "AirVittitoeBiggs.h"


SO_NODE_SOURCE(AirVittitoeBiggs)


void AirVittitoeBiggs::initClass()
{
    SO_NODE_INIT_CLASS(AirVittitoeBiggs, AirAbstract, "AirAbstract");
}

AirVittitoeBiggs::AirVittitoeBiggs()
{
    SO_NODE_CONSTRUCTOR(AirVittitoeBiggs);

    SO_NODE_DEFINE_ENUM_VALUE(Visibility, ClearDay);
    SO_NODE_DEFINE_ENUM_VALUE(Visibility, HazyDay);
    SO_NODE_SET_SF_ENUM_TYPE(visibility, Visibility);
    SO_NODE_ADD_FIELD( visibility, (ClearDay) );
    setName("Vittitoe-Biggs");
}

double AirVittitoeBiggs::transmission(double distance) const
{
    double d = distance/1000.;
    double t;
    if (visibility.getValue() == ClearDay)
        t = 0.99326 - 0.1046*d + 0.017*d*d - 0.002845*d*d*d;
    else
        t = 0.98707 - 0.2748*d + 0.03394*d*d;
    return t;
}

// Reference:
// Vittitoe, C.N., Biggs, F., 1978. Terrestrial propagation loss presented Amer. Sec. ISES meeting, Denver, August 1978. Sandia release SAND78-1137C, May 26.
// F. Biggs, C.N. Vittitoe The Helios model for the optical behaviour of reflecting solar concentrators SAND 76-0347 (March 1979)
//
// Default in SolarPilot and Delsol
