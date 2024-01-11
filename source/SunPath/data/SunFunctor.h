#pragma once

#include "SunPath/data/SkyModelPI.h"
#include "SunPath/calculators/SunCalculator.h"

namespace sp {


struct SunPath SunFunctor
{
    virtual double operator()(const vec3d& s) const = 0;
};


struct SunPath SunFunctorOne: SunFunctor
{
    double operator()(const vec3d& /*s*/) const {return 1.;}
};

struct SunPath SunFunctorPanelCos: SunFunctor
{
    vec3d n;
    double operator()(const vec3d& s) const;
};

struct SunPath SunFunctorHeliostatCos: SunFunctor
{
    vec3d t;
    double operator()(const vec3d& s) const;
};

struct SunPath SunFunctorHeliostatFieldOld: SunFunctor
{
    vec3d t;
    double operator()(const vec3d& s) const;
};

struct SunPath SunFunctorHeliostatField: SunFunctor
{
    vec3d t;
    double operator()(const vec3d& s) const;
};

struct SunPath SunFunctorDNI: SunFunctor
{
    SkyModelPI model;
    double operator()(const vec3d& s) const;
};

struct SunPath SunFunctorWeighted: SunFunctor
{
    SunFunctor* sw;
    SunFunctor* sf;
    double operator()(const vec3d& s) const;
};


} // namespace sp
