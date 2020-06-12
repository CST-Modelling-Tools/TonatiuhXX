#include "ShapeElliptic.h"

#include "kernel/profiles/ProfileRT.h"
#include "kernel/scene/TShapeKit.h"
#include "kernel/shape/DifferentialGeometry.h"
#include "libraries/geometry/Box3D.h"
#include "libraries/geometry/Ray.h"
using gcf::pow2;

SO_NODE_SOURCE(ShapeElliptic)


void ShapeElliptic::initClass()
{
    SO_NODE_INIT_CLASS(ShapeElliptic, ShapeRT, "ShapeRT");
}

ShapeElliptic::ShapeElliptic()
{
    SO_NODE_CONSTRUCTOR(ShapeElliptic);

    SO_NODE_ADD_FIELD( aX, (1.) );
    SO_NODE_ADD_FIELD( aY, (1.) );
    SO_NODE_ADD_FIELD( aZ, (1.) );
}

Box3D ShapeElliptic::getBox(ProfileRT* profile) const
{  
    Box3D box = profile->getBox();
    Vector3D v = box.absMax();
    double rX = aX.getValue();
    double rY = aY.getValue();
    double rZ = aZ.getValue();
    double s = 1. - pow2(v.x/rX) - pow2(v.y/rY);
    s = 1. - sqrt(s);
    box.pMax.z = s*rZ;
    return box;
}

bool ShapeElliptic::intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg, ProfileRT* profile) const
{
    double rZ = aZ.getValue();
    Vector3D g(1./aX.getValue(), 1./aY.getValue(), 1./rZ);
    Vector3D rayO = (ray.origin - Vector3D(0., 0., rZ))*g;
    Vector3D rayD = ray.direction()*g;

    double A = rayD.norm2();
    double B = 2.*dot(rayD, rayO);
    double C = rayO.norm2() - 1.;
    double ts[2];
    if (!gcf::solveQuadratic(A, B, C, &ts[0], &ts[1])) return false;

    for (int i = 0; i < 2; ++i)
    {
        double t = ts[i];
        if (t < ray.tMin + 1e-5 || t > ray.tMax) continue;

        Vector3D pHit = ray.point(t);
        if (pHit.z >= rZ) continue; // discard upper branch
        if (!profile->isInside(pHit.x, pHit.y)) continue;

        if (tHit == 0 && dg == 0)
            return true;
        else if (tHit == 0 || dg == 0)
            gcf::SevereError("ShapeElliptic::intersect");

        *tHit = t;
        dg->point = pHit;
        dg->u = pHit.x;
        dg->v = pHit.y;
        double s = 1. - pHit.z/rZ;
        dg->dpdu = Vector3D(1., 0., pHit.x/s*g.x*g.x*rZ);
        dg->dpdv = Vector3D(0., 1., pHit.y/s*g.y*g.y*rZ);
        dg->normal = Vector3D(-dg->dpdu.z, -dg->dpdv.z, 1.).normalized();
        dg->shape = this;
        dg->isFront = dot(dg->normal, ray.direction()) <= 0.;
        return true;
    }
    return false;
}

void ShapeElliptic::updateShapeGL(TShapeKit* parent)
{
    ProfileRT* profile = (ProfileRT*) parent->profileRT.getValue();

    Box3D box = profile->getBox();
    Vector3D q = box.absMin();
    Vector3D s = box.extent();

    double cx = aX.getValue()*aX.getValue()/std::abs(aZ.getValue());
    double cy = aY.getValue()*aY.getValue()/std::abs(aZ.getValue());
    double sx = 0.1*cx*(1. + pow2(q.x/cx));
    double sy = 0.1*cy*(1. + pow2(q.y/cy));
    int rows = 1 + ceil(s.x/sx);
    int columns = 1 + ceil(s.y/sy);

    makeQuadMesh(parent, QSize(rows, columns));
}

Vector3D ShapeElliptic::getPoint(double u, double v) const
{
    double rX = aX.getValue();
    double rY = aY.getValue();
    double rZ = aZ.getValue();
    double s = 1. - pow2(u/rX) - pow2(v/rY);
    s = 1. - sqrt(s);
    return Vector3D(u, v, s*rZ);
}

Vector3D ShapeElliptic::getNormal(double u, double v) const
{
    double rX = aX.getValue();
    double rY = aY.getValue();
    double rZ = aZ.getValue();
    double s = 1. - pow2(u/rX) - pow2(v/rY);
    return Vector3D(-u/(rX*rX), -v/(rY*rY), sqrt(s)/rZ).normalized();
}
