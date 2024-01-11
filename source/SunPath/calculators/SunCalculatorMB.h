#pragma once

#include "SunCalculator.h"

namespace sp {

class SunPath SunCalculatorMB: public SunCalculator
{
public:
    SunCalculatorMB();
    SunCalculatorMB* copy() const;

    Horizontal findHorizontalV(const QDateTime& t) const;

    QString info() const {return "Manuel Blanco (2001, 2020)";}

    enum Version {
        V2001,
        V2020
    };

    void setVersion(Version v) {m_version = v;}
    Version version() const {return m_version;}

private:
    Version m_version;
};

} // namespace sp
