#include "SunCalculatorET.h"

using namespace sp;


SunCalculatorET::SunCalculatorET()
{

}

SunCalculatorET* SunCalculatorET::copy() const
{
    return new SunCalculatorET(*this);
}

// accuracy about 1 degree
Horizontal SunCalculatorET::findHorizontalV(const QDateTime& t) const
{
    QDateTime ut = t.toUTC();
    int tD = ut.date().dayOfYear();
    double tH = ut.time().msecsSinceStartOfDay()/double(1000*60*60);

    double lambda = 2*pi*(tD - 81)/365.;
    double eot = findEOT(lambda);
    double omega = 2*pi*(tH - 12)/24 + m_location.longitude() - eot;
    double delta = asin(sin(lambda)*sin(s_obliquity));

    Equatorial ec(omega, delta);
    return findHorizontal(ec);
}

QDateTime SunCalculatorET::findTime(const Horizontal& hc, const QDateTime& t0) const
{
    Equatorial hd = findEquatorial(hc);
    double sinLambda = sin(hd.declination())/sin(s_obliquity);
    sinLambda = clamp(sinLambda, -1., 1.);
    double lambda = asin(sinLambda);
    int tD0 = t0.date().dayOfYear();
    if (tD0 - 81 > 365/4. && tD0 - 81 < 3*365/4.)
        lambda = pi - lambda;

    double eot = findEOT(lambda);
    double tH = (hd.hour() - m_location.longitude() + eot)*24/(2*pi) + 12.;
    tH = normalizePeriodic(tH, 0., 24.);

    float tD = round(lambda/(2*pi)*365. + 81);
    tD = normalizePeriodic(tD, 0., 365.);

    QDate date = t0.date().addDays(tD - tD0);
    QTime time = QTime::fromMSecsSinceStartOfDay(tH*1000*60*60);
    QDateTime t(date, time, Qt::UTC);
    t = t.toTimeSpec(t0.timeSpec());
    return t;
}

double SunCalculatorET::findEOT(double lambda) const
{
    double eot = 7.53*cos(lambda) + 1.5*sin(lambda) - 9.87*sin(2*lambda); // in minutes
    return (2*pi*eot)/(24*60);
}
