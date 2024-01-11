#pragma once

#include "SunPath/math/math.h"
#include "Summator.h"

namespace sp {

class SunPath ErrorAnalysis
{

public:
    ErrorAnalysis():
        m_counter(0),
        m_xAbsMax(0.),
        m_fMax(0.)
    {}

    void push(double x);
    void push(double f, double f0);
    void operator<<(double x) {push(x);}

    double mean() const {return m_summator.result()/m_counter;}
    double meanAbs() const {return m_summatorAbs.result()/m_counter;}
    double rms() const {return std::sqrt(m_summatorSq.result()/m_counter);}
    double maxAbs() const {return m_xAbsMax;}
    double fMax() const {return m_fMax;}

protected:
    int m_counter;
    Summator m_summator;
    Summator m_summatorAbs;
    Summator m_summatorSq;
    double m_xAbsMax;
    double m_fMax;
};

} // namespace sp
