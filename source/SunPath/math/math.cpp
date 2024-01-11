#include "math.h"

#include <algorithm>

namespace sp {

// normalize x to the range [x0, x0 + p)
double normalizePeriodic(double x, double x0, double p)
{
    return x - p*floor((x - x0)/p);
}

bool solveQuadratic(double a, double b, double c, double* x0, double* x1)
{
    double d = b*b - 4.*a*c;
    if (d < 0.) return false;

    double q = -0.5;
    if (b > 0.)
        q *= b + sqrt(d);
    else
        q *= b - sqrt(d);

    *x0 = q/a;
    *x1 = c/q;
    if (*x0 > *x1) std::swap(*x0, *x1);
    return true;
}

} // namespace sp
