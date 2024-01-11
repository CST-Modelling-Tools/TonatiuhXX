#include "Interval.h"

namespace sp {

Interval::Interval():
    m_a(infinity),
    m_b(-infinity)
{

}

Interval::Interval(double a, double b):
    m_a(a),
    m_b(b)
{

}

void Interval::expand(double delta)
{
    m_a -= delta;
    m_b += delta;
}

void Interval::operator<<(double x)
{
    m_a = std::min(m_a, x);
    m_b = std::max(m_b, x);
}

void Interval::operator<<(const Interval& other)
{
    m_a = std::min(m_a, other.m_a);
    m_b = std::max(m_b, other.m_b);
}

QVector<double> Interval::sampleDivisions(int divs)
{
    QVector<double> ans;
    ans << m_a;

    if (divs > 1) {
        double step = (m_b - m_a)/divs;
        for (int n = 1; n < divs; ++n)
            ans << m_a + n*step;
    }

    if (m_a != m_b) ans << m_b;
    return ans;
}

QVector<double> Interval::sampleResolution(double res)
{
    int divs = round((m_b - m_a)/res);
    return sampleDivisions(divs);
}

} // namespace sp
