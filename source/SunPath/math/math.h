#define _USE_MATH_DEFINES
#include <cmath>

#pragma once

#include "SunPath/SunPath.h"

#include <limits>

namespace sp {

    const double pi = M_PI;
    const double degree = pi/180;
    const double infinity = std::numeric_limits<double>::infinity();

    template<class T>
    bool eqz(T x)
    {
        return std::abs(x) < std::numeric_limits<T>::epsilon();
    }

    template<class T>
    bool equals(T x, T y)
    {
        return std::abs(x - y) < std::numeric_limits<T>::epsilon();
    }

    template<class T>
    T clamp(T x, T a, T b)
    {
        if (x <= a) return a;
        if (x >= b) return b;
        return x;
    }

    template<class T, class U>
    T lerp(T a, T b, U u)
    {
        return (U(1) - u)*a + u*b;
    }

    template<class T>
    T pow2(T x) {return x*x;}

    SunPath double normalizePeriodic(double x, double x0, double p);

    inline double normalizeAngle(double phi, double phi0)
    {
        return normalizePeriodic(phi, phi0, 2*pi);
    }

    SunPath bool solveQuadratic(double a, double b, double c, double* x0, double* x1);

} // namespace sp
