#include "Summator.h"

namespace sp {

Summator::Summator()
{
    reset();
}

void Summator::reset()
{
    m_correction = 0.;
    m_result = 0.;
}

void Summator::add(double x)
{
    double term = x - m_correction;
    double temp = m_result + term;
    m_correction = (temp - m_result) - term;
    m_result = temp;
}

} // namespace sp
