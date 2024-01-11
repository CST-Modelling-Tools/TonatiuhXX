#include "SunCalculatorNREL.h"

#include "NREL/spa.h"
#include <QDebug>

using namespace sp;


SunCalculatorNREL::SunCalculatorNREL()
{

}

SunCalculatorNREL* SunCalculatorNREL::copy() const
{
    return new SunCalculatorNREL(*this);
}

Horizontal SunCalculatorNREL::findHorizontalV(const QDateTime& t) const
{
    QDate date = t.date();
    QTime time = t.time();

    spa_data spa;
    spa.year          = date.year();
    spa.month         = date.month();
    spa.day           = date.day();
    spa.hour          = time.hour();
    spa.minute        = time.minute();
    spa.second        = time.second();
    spa.timezone      = t.offsetFromUtc()/3600.;
    spa.delta_ut1     = 0.;
    spa.delta_t       = 0.;
    spa.longitude     = m_location.longitude()/degree;
    spa.latitude      = m_location.latitude()/degree;
    spa.elevation     = 0.;
    spa.pressure      = 1000.;
    spa.temperature   = 20.;
    spa.slope         = 0.;
    spa.azm_rotation  = 0.;
    spa.atmos_refract = 0.5667;
    spa.function      = SPA_ZA;

    int result = spa_calculate(&spa);
    if (result != 0)
        qDebug("SunCalculatorNREL error code: %d", result);

    return Horizontal(
        spa.azimuth*degree, (90. - spa.zenith)*degree
    );
}
