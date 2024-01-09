#include "SunGaussian.h"

#include <Inventor/sensors/SoNodeSensor.h>
#include "libraries/math/gcf.h"


SO_NODE_SOURCE(SunGaussian)

void SunGaussian::initClass()
{
    SO_NODE_INIT_CLASS(SunGaussian, SunShape, "SunShape");
}

SunGaussian::SunGaussian()
{
    SO_NODE_CONSTRUCTOR(SunGaussian);
    SO_NODE_ADD_FIELD( sigma, (2.51e-3) );

    updateSigma();

    m_sensor = new SoNodeSensor(onSensor, this);
    m_sensor->setPriority(0); // does not help?
    m_sensor->attach(this);
}

SunGaussian::~SunGaussian()
{
    delete m_sensor;
}

SoNode* SunGaussian::copy(SbBool copyConnections) const
{
    SunGaussian* sun = dynamic_cast<SunGaussian*>(SoNode::copy(copyConnections));
    sun->m_thetaMax = m_thetaMax;
    sun->m_h = m_h;
    return sun;
}

vec3d SunGaussian::generateRay(Random& rand) const
{
    double phi = gcf::TwoPi*rand.RandomDouble();
    double theta = zenithAngle(rand);
    double sinTheta = sin(theta);
    double cosTheta = cos(theta);
    double cosPhi = cos(phi);
    double sinPhi = sin(phi);

    return vec3d(
        sinTheta*cosPhi,
        sinTheta*sinPhi,
        cosTheta
    );
}

double SunGaussian::getThetaMax() const
{
    return m_thetaMax;
}

double SunGaussian::shape(double theta) const
{
    double q = theta/sigma.getValue();
    return exp(-q*q/2.);
}

void SunGaussian::updateSigma()
{
    double s = sigma.getValue();
    m_thetaMax = 10.*s;
    double xMax = s; // first-order approximation
    m_h = 1.01*pdfTheta(xMax);
}

double SunGaussian::zenithAngle(Random& rand) const
{
    double theta;
    double value;

    // acceptance-rejection method
    // split in two rects like Buie
    do {
        theta = rand.RandomDouble()*m_thetaMax;
        value = rand.RandomDouble()*m_h;
    } while (value > pdfTheta(theta));

    return theta;
}

double SunGaussian::pdfTheta(double theta) const
{
    return shape(theta)*sin(theta)*cos(theta);
}

void SunGaussian::onSensor(void* data, SoSensor*)
{
    SunGaussian* sun = (SunGaussian*) data;
    double sigma = sun->sigma.getValue();
    if (sigma < 0.) sun->sigma = 2.51e-3;
    sun->updateSigma();
}

// References:
// F. J. Collado and J. Guallar, "Fast and reliable flux map on cylindrical receivers," Solar Energy 169, 556�564 (2018).
