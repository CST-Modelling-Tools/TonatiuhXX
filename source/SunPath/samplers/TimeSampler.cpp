#include "TimeSampler.h"

#include "SunPath/samplers/SunTemporal.h"

namespace sp {


double toHours(QDateTime dt)
{
    double h = (dt.date().dayOfYear() - 1)*24;
    h += dt.time().msecsSinceStartOfDay()/1000./3600.;
    return h;
}


TimeSampler::TimeSampler(SunTemporal* sunTemporal):
    m_sunTemporal(sunTemporal)
{

}

bool TimeSampler::sample(const QVector<QDateTime>& ts, bool adjustDay)
{
    QVector<TimeStamp> ans = makeSamples(ts);
    if (adjustDay) adjustSamples(ans);
    m_sunTemporal->setTimeStamps(ans);
    return true;
}

bool TimeSampler::sample(QDateTime tA, QDateTime tB, QTime tStep, bool adjustDay)
{
    if (tA >= tB) return false;

    QVector<TimeStamp> ans = makeSamples(tA, tB, tStep);
    if (adjustDay) adjustSamples(ans);
    m_sunTemporal->setTimeStamps(ans);
    return true;
}

bool TimeSampler::resample(QTime tStep, bool adjustDay)
{
    QDateTime tBegin = m_sunTemporal->timeStamps().first().t;
    QDateTime tEnd = m_sunTemporal->timeStamps().last().t;

    if (tBegin.date().year() != tEnd.date().year()) {
        int y = tBegin.date().year();
        int m = tEnd.date().month();
        int d = tEnd.date().day();
        tEnd.setDate(QDate(y, m, d));
    }

    QVector<TimeStamp> ans = makeSamples(tBegin, tEnd, tStep);
    if (adjustDay) adjustSamples(ans);

    QVector<double> data;
    for (int n = 1; n < ans.size(); ++n)
        data << m_sunTemporal->average(ans[n - 1].t, ans[n].t);

    m_sunTemporal->setTimeStamps(ans);
    m_sunTemporal->setData(data);
    return true;
}

QVector<TimeStamp> TimeSampler::makeSamples(const QVector<QDateTime>& ts)
{
    QVector<TimeStamp> ans;

    SunCalculator* sc = m_sunTemporal->calculator();
    double th = toHours(ts[0]);
    double dt = toHours(ts[1]) - th;
    for (const QDateTime& t : ts) {
        ans << TimeStamp(t, sc->findVector(t), th);
        th += dt;
//        ans << TimeStamp(t, sc->findVector(t), toHours(t));
//        qDebug() << t << " " << sc->findHorizontalV(t).elevation()/sp::degree << toHours(t);
    }
    return ans;
}

QVector<TimeStamp> TimeSampler::makeSamples(QDateTime tA, QDateTime tB, QTime tStep)
{
    QVector<TimeStamp> ans;

    int dt = tStep.msecsSinceStartOfDay();
    SunCalculator* sc = m_sunTemporal->calculator();
    for (QDateTime t = tA; t <= tB; t = t.addMSecs(dt))
        ans << TimeStamp(t, sc->findVector(t), toHours(t));

    return ans;
}

void TimeSampler::adjustSamples(QVector<TimeStamp>& data)
{
    int dth = data[0].t.msecsTo(data[1].t)/1000./3600.;
    double s0 = sin(-0.25*degree); // sun below horizon

    vec3d sA = data[0].s;
    for (int n = 1; n < data.size(); ++n) {
        vec3d sB = data[n].s;

        if (sA.z < s0 && s0 < sB.z) // sunrise
            data[n - 1].tc += (s0 - sA.z)/(sB.z - sA.z)*dth;
        else if (sA.z > s0 && s0 > sB.z) // sunset
            data[n].tc -= (s0 - sB.z)/(sA.z - sB.z)*dth;

        sA = sB;
    }
}


} // namespace sp
