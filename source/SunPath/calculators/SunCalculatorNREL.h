#pragma once

#include "calculators/SunCalculator.h"

namespace sp {

class SunPath SunCalculatorNREL: public SunCalculator
{
public:
    SunCalculatorNREL();
    SunCalculatorNREL* copy() const;

    Horizontal findHorizontalV(const QDateTime& t) const;

    QString info() const {return "NREL (2003)";}
};

} // namespace sp
