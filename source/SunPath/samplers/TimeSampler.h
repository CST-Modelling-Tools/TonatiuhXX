#pragma once

#include "SunPath/calculators/SunCalculator.h"

namespace sp {


class SunTemporal;


double SunPath toHours(QDateTime dt);


struct SunPath TimeStamp
{
    TimeStamp() {}
    TimeStamp(QDateTime t, vec3d s, double tc):
        t(t), s(s), tc(tc) {}

    QDateTime t; // time
    vec3d s; // vector to sun
    double tc; // time in hours corrected for sunrise and sunset
};


class SunPath TimeSampler
{
public:
    TimeSampler(SunTemporal* sunTemporal);

    bool sample(const QVector<QDateTime>& ts, bool adjustDay = false);
    bool sample(QDateTime tA, QDateTime tB, QTime tStep = QTime(1, 00), bool adjustDay = false);
    bool resample(QTime tStep = QTime(1, 00), bool adjustDay = false);

protected:
    QVector<TimeStamp> makeSamples(const QVector<QDateTime>& ts);
    QVector<TimeStamp> makeSamples(QDateTime tA, QDateTime tB, QTime tStep = QTime(1, 00));
    void adjustSamples(QVector<TimeStamp>& data);

protected:
    SunTemporal* m_sunTemporal;
};


} // namespace sp
