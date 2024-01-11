#pragma once

#include "SunPath/data/SkyModel.h"

namespace sp {

class SunPath SkyModelPI: public SkyModel
{
public:
    SkyModelPI();
    SkyModelPI* copy() const;

    double findDNI(double alpha) const;
    double findDHI(double alpha) const;
    double findGHI(double alpha) const;

    QString info() {return "Pierre Ineichen (2001)";}

private:
   double m_i0;

   double m_taub;
   double m_b;

   double m_taud;
   double m_d;

   double m_taug;
   double m_g;
};

} // namespace sp
