#pragma once

#include "SunPath/samplers/SkySampler.h"

namespace sp {


typedef double (*KernelFunc)(void* k, const SkyNode& sn, const vec3d& r);

template<class T>
inline double kernelFuncT(void* k, const SkyNode& sn, const vec3d& r) {
    return (*(T*)k)(sn, r);
}

struct SunPath SkyKernel
{
    SkyKernel() {kf = kernelFuncT<SkyKernel>;}
    KernelFunc kf;
    inline double kernel(const SkyNode& sn, const vec3d& r) {return kf(this, sn, r);}
    double operator()(const SkyNode& /*sn*/, const vec3d& /*r*/) {return 0.;}
};

struct SunPath SkyKernelGaussian3D: SkyKernel
{
    SkyKernelGaussian3D() {kf = kernelFuncT<SkyKernelGaussian3D>;}
    double operator()(const SkyNode& sn, const vec3d& r) const;
};

struct SunPath SkyKernelPolyharmonic: SkyKernel
{
    SkyKernelPolyharmonic(int order): order(order) {kf = kernelFuncT<SkyKernelPolyharmonic>;}
    double operator()(const SkyNode& sn, const vec3d& r) const;

    int order;
};


} // namespace sp
