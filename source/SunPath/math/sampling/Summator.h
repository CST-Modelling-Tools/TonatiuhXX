#pragma once

#include <SunPath/SunPath.h>

namespace sp {

// Kahan
class SunPath Summator
{
public:
    Summator();

    void reset();

    void add(double x);
    Summator& operator<<(double x) {add(x); return *this;}
    Summator& operator+=(double x) {add(x); return *this;}

    double result() const {return m_result;}
    operator double() const {return m_result;}

protected:
    double m_correction;
    double m_result;
};

} // namespace sp
