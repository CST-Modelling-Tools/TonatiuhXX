#include "SunCalculatorRG.h"

#include "RG/Sun_position_algorithms.h"

using namespace sp;


SunCalculatorRG::SunCalculatorRG()
{

}

SunCalculatorRG* SunCalculatorRG::copy() const
{
    return new SunCalculatorRG(*this);
}

Horizontal SunCalculatorRG::findHorizontalV(const QDateTime& t) const
{
    QDate date = t.date();
    QTime time = t.time();

    sunpos sp;
    sp.UT = time.hour() + time.minute()/60. + time.second()/3600. - t.offsetFromUtc()/3600.;
    sp.Day = date.day();
    sp.Month = date.month();
    sp.Year = date.year();
    sp.Dt = 0.;
    sp.Longitude = m_location.longitude();
    sp.Latitude = m_location.latitude();
    sp.Pressure = 1.;
    sp.Temperature = 20.;

    sp.Algorithm5();
    return Horizontal(
        sp.Azimuth - 180.*degree, 90.*degree - sp.Zenith
    );
}
