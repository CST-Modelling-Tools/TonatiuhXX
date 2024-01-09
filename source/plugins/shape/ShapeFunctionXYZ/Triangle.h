#pragma once

#include "libraries/math/3D/Box3D.h"

struct DifferentialGeometry;


class Triangle
{

public:
    Triangle(
        const vec3d& pA, const vec3d& pB, const vec3d& pC,
        const vec3d& nA, const vec3d& nB, const vec3d& nC
    );

    const vec3d& pA() const {return m_pA;}
    const vec3d& pB() const {return m_pB;}
    const vec3d& pC() const {return m_pC;}

    const vec3d& nA() const {return m_nA;}
    const vec3d& nB() const {return m_nB;}
    const vec3d& nC() const {return m_nC;}

    const Box3D& box() const {return m_box;}
    vec3d center() const {return m_center;}

    bool intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg) const;

private:  
    vec3d m_pA, m_pB, m_pC;
    vec3d m_nA, m_nB, m_nC;

    Box3D m_box;
    vec3d m_center;

    vec3d m_eu, m_ev;
    double m_tolerance;
};

