#include "SkyKernels.h"

namespace sp {


double SkyKernelGaussian3D::operator()(const SkyNode& sn, const vec3d& r) const
{
    double arg = dot(sn.v, r) - 1.;
    arg *= sn.coeff;
    if (arg < -20.) return 0.;
    return exp(arg);
}

// https://en.wikipedia.org/wiki/Polyharmonic_spline
double SkyKernelPolyharmonic::operator()(const SkyNode& sn, const vec3d& r) const
{
    double rho = (sn.v - r).norm2();
    rho = pow(rho, order/2.);
    if (order%2 == 1) {
        return rho;
    } else {
        if (rho > 1.)
            return rho*log(rho);
        else
            return log(pow(rho, rho));
    }
}

//SkyKernel* sk = new SkyKernelPolyharmonic(7);
// without virtual, faster?
//double x = sk->kernel(SkyNode(), vec3d::Zero);
// with virtual
//double x = sk(SkyNode(), vec3d::Zero); // add virtual to operator()


} // namespace sp
