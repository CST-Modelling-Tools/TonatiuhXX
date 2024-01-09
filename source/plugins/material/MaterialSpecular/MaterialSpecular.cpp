#include "MaterialSpecular.h"

#include <Inventor/sensors/SoNodeSensor.h>

#include "libraries/math/gcf.h"
#include "libraries/math/3D/Ray.h"
#include "libraries/math/3D/Transform.h"
#include "kernel/shape/DifferentialGeometry.h"
#include "kernel/random/Random.h"
#include "kernel/node/TonatiuhFunctions.h"


SO_NODE_SOURCE(MaterialSpecular)

void MaterialSpecular::initClass()
{
    SO_NODE_INIT_CLASS(MaterialSpecular, MaterialRT, "MaterialRT");
}

MaterialSpecular::MaterialSpecular()
{
    SO_NODE_CONSTRUCTOR(MaterialSpecular);
    isBuiltIn = TRUE;

    SO_NODE_ADD_FIELD(reflectivity, (0.95) ); // 1 may give infinite reflections

    SO_NODE_DEFINE_ENUM_VALUE(Distribution, pillbox);
    SO_NODE_DEFINE_ENUM_VALUE(Distribution, Gaussian);
    SO_NODE_SET_SF_ENUM_TYPE(distribution, Distribution);
    SO_NODE_ADD_FIELD(distribution, (Gaussian) );

    SO_NODE_ADD_FIELD(slope, (0.002) ); // in radians

    m_sensor = new SoNodeSensor(onSensor, this);
    m_sensor->attach(this);
}

MaterialSpecular::~MaterialSpecular()
{
    delete m_sensor;
}

bool MaterialSpecular::OutputRay(const Ray& rayIn, const DifferentialGeometry& dg, Random& rand, Ray& rayOut) const
{
    // reflectivity
    if (rand.RandomDouble() >= reflectivity.getValue()) return false;

    rayOut.origin = dg.point;

    vec3d normal;
    double sigma = slope.getValue();
    if (sigma > 0.) {
        if (distribution.getValue() == Distribution::pillbox)
        {
            double phi = gcf::TwoPi*rand.RandomDouble();
            double sinTheta = sin(sigma)*sqrt(rand.RandomDouble());
            double cosTheta = sqrt(1. - sinTheta*sinTheta);
            normal.x = sinTheta*cos(phi);
            normal.y = sinTheta*sin(phi);
            normal.z = cosTheta;
        }
        else //if (distribution.getValue() == Distribution::Gaussian)
        {
            // https://en.wikipedia.org/wiki/Marsaglia_polar_method
            double u, v, s;
            do {
                u = 2.*rand.RandomDouble() - 1.;
                v = 2.*rand.RandomDouble() - 1.;
                s = u*u + v*v;
            } while (s > 1. || s == 0.);
            s = sigma*sqrt(-2.*log(s)/s);

            normal.x = s*u;
            normal.y = s*v;
            normal.z = 1.;
        }
        vec3d vx = dg.dpdu.normalized();
        vec3d vy = dg.dpdv.normalized();
        vec3d vz = dg.normal.normalized(); // always normalized?
        normal = vx*normal.x + vy*normal.y + vz*normal.z;
        normal.normalize();
    } else
        normal = dg.normal;

    vec3d d = rayIn.direction().reflected(normal);
    rayOut.setDirection(d); // double sided
    return true;
}

void MaterialSpecular::onSensor(void* data, SoSensor*)
{
    MaterialSpecular* material = (MaterialSpecular*) data;
    if (material->reflectivity.getValue() < 0.)
        material->reflectivity = 0.;
    if (material->reflectivity.getValue() > 1.)
        material->reflectivity = 1.;
}
