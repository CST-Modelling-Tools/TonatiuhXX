#include "ErrorAnalysis.h"

namespace sp {


void ErrorAnalysis::push(double x)
{
    m_counter++;
    m_summator += x;
    x = std::abs(x);
    m_summatorAbs += x;
    m_summatorSq += x*x;
    if (x > m_xAbsMax)
        m_xAbsMax = x;
}

void ErrorAnalysis::push(double f, double f0)
{
    push(f - f0);
    f0 = std::abs(f0);
    if (f0 > m_fMax) m_fMax = f0;
}


} // namespace sp
