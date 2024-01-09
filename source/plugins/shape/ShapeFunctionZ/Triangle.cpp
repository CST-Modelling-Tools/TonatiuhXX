#include "Triangle.h"

#include "kernel/shape/DifferentialGeometry.h"
#include "libraries/math/3D/Ray.h"


Triangle::Triangle(
    const vec3d& pA, const vec3d& pB, const vec3d& pC,
    const vec3d& nA, const vec3d& nB, const vec3d& nC
):
    m_pA(pA), m_pB(pB), m_pC(pC),
    m_nA(nA), m_nB(nB), m_nC(nC)
{
    m_box << pA;
    m_box << pB;
    m_box << pC;
    m_center = m_box.center();

    m_eu = m_pA - m_pC;
    m_ev = m_pB - m_pC;
    m_tolerance = m_eu.norm()*m_ev.norm()*1e-6;
}

bool Triangle::intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg) const
{
    // point
    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    vec3d qv = cross(ray.direction(), m_ev);
    double det = dot(m_eu, qv);
    if (std::abs(det) < m_tolerance) return false;
    double detInv = 1./det;

    vec3d qt = ray.origin - m_pC;
    double u = dot(qv, qt)*detInv;
    if (u < 0. || u > 1.) return false;

    vec3d qu = cross(qt, m_eu);
    double v = dot(qu, ray.direction())*detInv;
    if (v < 0. || u + v > 1.) return false;

    double t = dot(qu, m_ev)*detInv;
    if (t < ray.tMin + m_tolerance || t > ray.tMax) return false;
    vec3d pHit = ray.point(t);

    // normal
    vec3d vN = u*m_nA + v*m_nB + (1. - u - v)*m_nC;
    vN.normalize();
    vec3d vU = vN.findOrthogonal().normalize();
    vec3d vV = cross(vN, vU);

    *tHit = t;
    dg->point = pHit;
    dg->uv = vec2d(u, v);
    dg->dpdu = vU;
    dg->dpdv = vV;
    dg->normal = vN;
    dg->shape = 0;
    dg->isFront = dot(vN, ray.direction()) <= 0.;
    return true;
}
