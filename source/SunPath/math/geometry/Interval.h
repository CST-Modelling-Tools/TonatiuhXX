#pragma once

#include "SunPath/math/math.h"

#include <QVector>

namespace sp {

class SunPath Interval
{
public:
    Interval();
    Interval(double a, double b);
    bool isValid() const {return m_a <= m_b;}

    double min() const {return m_a;}
    double max() const {return m_b;}
    double size() const {return m_b - m_a;}
    double mid() const {return (m_a + m_b)/2.;}

    void expand(double delta);
    void operator<<(double );
    void operator<<(const Interval& other);

    bool isInside(double x) const {return m_a <= x && x <= m_b;}
    bool intersect(const Interval& other) const {return m_a <= other.m_b && other.m_a <= m_b;}

    double toNormalized(double x) const {return (x - m_a)/(m_b - m_a);}
    double fromNormalized(double u) const {return (1. - u)*m_a + u*m_b;}

    QVector<double> sampleDivisions(int divs);
    QVector<double> sampleResolution(double res);

protected:
    double m_a;
    double m_b;
};

} // namespace sp
