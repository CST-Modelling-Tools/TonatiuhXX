#pragma once

#include "SunCalculator.h"

namespace sp {

class SunPath SunCalculatorET: public SunCalculator
{
public:
    SunCalculatorET();
    SunCalculatorET* copy() const;

    Horizontal findHorizontalV(const QDateTime& t) const;
    QDateTime findTime(const Horizontal& hc, const QDateTime& t0) const;

    double findEOT(double lambda) const;

    QString info() const {return "Equation of time";}
};

} // namespace sp
