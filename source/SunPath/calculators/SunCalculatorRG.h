#pragma once

#include "SunCalculator.h"

namespace sp {

class SunPath SunCalculatorRG: public SunCalculator
{
public:
    SunCalculatorRG();
    SunCalculatorRG* copy() const;

    Horizontal findHorizontalV(const QDateTime& t) const;

    QString info() const {return "Roberto Grena (2012)";}
};

} // namespace sp
