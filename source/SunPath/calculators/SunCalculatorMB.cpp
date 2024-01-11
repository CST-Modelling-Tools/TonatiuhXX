#include "SunCalculatorMB.h"

#include "MB/sunpos.h"

using namespace sp;


SunCalculatorMB::SunCalculatorMB()
{
    setVersion(Version::V2020);
}

SunCalculatorMB* SunCalculatorMB::copy() const
{
    return new SunCalculatorMB(*this);
}

Horizontal SunCalculatorMB::findHorizontalV(const QDateTime& t) const
{
    cLocation locationMB;
    locationMB.dLongitude = m_location.longitude()/degree;
    locationMB.dLatitude = m_location.latitude()/degree;

    cTime timeMB;
    QDateTime ut = t.toUTC();
    timeMB.iYear = ut.date().year();
    timeMB.iMonth = ut.date().month();
    timeMB.iDay = ut.date().day();
    timeMB.dHours = ut.time().hour();
    timeMB.dMinutes = ut.time().minute();
    timeMB.dSeconds = ut.time().second();

    cSunCoordinates coordsMB;
    if (m_version == Version::V2001)
        sunpos(timeMB, locationMB, &coordsMB);
    else
        sunpos2(timeMB, locationMB, &coordsMB);

    return Horizontal(
        coordsMB.dAzimuth*degree, (90. - coordsMB.dZenithAngle)*degree
    );
}
