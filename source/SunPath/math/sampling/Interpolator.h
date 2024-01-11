#pragma once

#include "SunPath/math/geometry/vec2d.h"

#include <QVector>

namespace sp {

// data should be sorted
class SunPath Interpolator
{
public:
    Interpolator() {}
    virtual ~Interpolator() {}

    const QVector<vec2d>& data() const {return m_data;}
    virtual void setData(const QVector<vec2d>& data) {m_data = data;}
    int indexA(double x) const;

    double function(double x) const;
    virtual double functionA(int iA, double x) const;

    double derivative(double x) const;
    virtual double derivativeA(int iA, double x) const;

protected:
     QVector<vec2d> m_data;
};


class SunPath InterpolatorLinear: public Interpolator
{
public:
    InterpolatorLinear() {}
    void setData(const QVector<vec2d>& data);

    double functionA(int iA, double x) const;
    double derivativeA(int iA, double x) const;

    const QVector<double>& derivatives() const {return m_derivatives;}

protected:
    QVector<double> m_derivatives;
};


// linear derivative, monotonic quadratic
// class SunPath InterpolatorLinearD: public Interpolator


// https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Catmull%E2%80%93Rom_spline
class SunPath InterpolatorCubic: public Interpolator
{
public:
    InterpolatorCubic() {}
    void setData(const QVector<vec2d>& data);

    double functionA(int iA, double x) const;
    double derivativeA(int iA, double x) const;
    void coeffs(int iA, double* a) const;

private:
    QVector<double> m_m;
};


// https://en.wikipedia.org/wiki/Monotone_cubic_interpolation
class SunPath InterpolatorCubicMono: public Interpolator
{
public:
    InterpolatorCubicMono() {}
    void setData(const QVector<vec2d>& data);
    const QVector<double>& derivatives() const {return m_m;}

    double functionA(int iA, double x) const;
    double derivativeA(int iA, double x) const;

private:
    QVector<double> m_m;
    QVector<double> m_c2;
    QVector<double> m_c3;
};


} // namespace sp
