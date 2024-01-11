#include "SunTemporal.h"

#include "SunPath/math/sampling/Summator.h"
#include "SunPath/math/sampling/Interpolator.h"

namespace sp {


// assumptions:
// the duration of periods are the same
// the months can be taken from different years
// the sample is at the end of time interval (averaged during the interval)

// interpolated time in hours
// year ignored
// accumulated values
// zero initial condition

// integration
// in hours

// angular sun diameter 0.5 deg
// motion 360/24 = 15 degrees in hour
// sunrise at -0.25 deg elevation (simple)
// if tA < sunrise < tB, move tA to sunrise
// if tA < sunset < tB, move tB to sunset

// cache previous index?
// for faster search

// https://www.nrel.gov/docs/fy08osti/43156.pdf
// "during period ending at the timestamp" stamp = 1 (default)
// "during period starting at the timestamp" stamp = 0


SunTemporal::SunTemporal(SunCalculator& calculator):
    m_calculator(calculator.copy()),
    m_interpolator(new InterpolatorLinear)
{

}

void SunTemporal::setTimeStamps(const QVector<TimeStamp>& timeStamps)
{
    m_timeStamps = timeStamps;
    m_timeStepH = timeStamps[0].t.msecsTo(timeStamps[1].t)/1000./3600.;
}

void SunTemporal::setData(const QVector<double>& data)
{
    Q_ASSERT(data.size() == m_timeStamps.size() - 1);
    m_data = data;

    QVector<vec2d> points;
    Summator sum;
    points << vec2d(m_timeStamps[0].tc, 0.);
    for (int n = 1; n < m_timeStamps.size(); ++n) {
        sum += m_data[n - 1]*m_timeStepH;
        points << vec2d(m_timeStamps[n].tc, sum.result());
    }
    m_interpolator->setData(points);
}

void SunTemporal::setData(const SunFunctor& sf)
{
    QVector<double> temp;
    for (const TimeStamp& ts : m_timeStamps)
        temp << sf(ts.s);

    QVector<double> data;
    for (int n = 1; n < temp.size(); ++n)
        data << (temp[n - 1] + temp[n])/2.;

    setData(data);
}

double SunTemporal::interpolate(QDateTime t) const
{
    double x = toHours(t);
    return m_interpolator->derivative(x);
}

double SunTemporal::integrate() const
{
    return m_interpolator->data().last().y;
}

double SunTemporal::integrate(QDateTime tA, QDateTime tB) const
{
    double xA = toHours(tA);
    double yA = m_interpolator->function(xA);
    double xB = toHours(tB);
    double yB = m_interpolator->function(xB);
    return yB - yA;
}

double SunTemporal::average() const
{
    const vec3d& p = m_interpolator->data().last();
    return p.y/p.x;
}

double SunTemporal::average(QDateTime tA, QDateTime tB) const
{
    double xA = toHours(tA);
    double yA = m_interpolator->function(xA);
    double xB = toHours(tB);
    double yB = m_interpolator->function(xB);
    return (yB - yA)/(xB - xA);
}

double SunTemporal::integrateWeighted(const SunFunctor& sf) const
{
    Summator sum;
    for (int n = 1; n < m_timeStamps.size(); ++n) {
        double w = m_data[n - 1];
        double f = sf(m_timeStamps[n - 1].s) + sf(m_timeStamps[n].s);
        sum += w*f/2.;
    }
    return sum.result()*m_timeStepH;

    // derivatives
//    QVector<double> fs;
//    for (const TimeStamp& ts : m_timeStamps)
//        fs << sf(ts.s);

//    const QVector<double>& ws = m_interpolator->derivatives();

//    Summator sum;
//    sum += fs[0]*ws[0]/2.;
//    int nMax = fs.size() - 1;
//    for (int n = 1; n < nMax; ++n)
//        sum += fs[n]*ws[n];
//    sum += fs[nMax]*ws[nMax]/2.;

//    return sum.result()*m_timeStepH;
}


} // namespace sp
