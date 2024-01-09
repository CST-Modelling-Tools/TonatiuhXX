#include "ShapeFunctionXYZ.h"

#include <QJSEngine>

#include <QDebug>

#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>

#include "kernel/profiles/ProfileRT.h"
#include "kernel/profiles/ProfilePolygon.h"
#include "libraries/DistMesh/PolygonMesh.h"
#include "kernel/scene/TShapeKit.h"
#include "kernel/shape/DifferentialGeometry.h"
#include "libraries/math/3D/Box3D.h"
#include "libraries/math/3D/Ray.h"
#include "kernel/node/TonatiuhFunctions.h"
using gcf::pow2;

SO_NODE_SOURCE(ShapeFunctionXYZ)


void ShapeFunctionXYZ::initClass()
{
    SO_NODE_INIT_CLASS(ShapeFunctionXYZ, ShapeRT, "ShapeRT");
}

ShapeFunctionXYZ::ShapeFunctionXYZ()
{  
    m_bvh = 0;
    SO_NODE_CONSTRUCTOR(ShapeFunctionXYZ);

    SO_NODE_ADD_FIELD( functionX, ("u") );
    SO_NODE_ADD_FIELD( functionY, ("v") );
    SO_NODE_ADD_FIELD( functionZ, ("(u*u + v*v)/4") );
    SO_NODE_ADD_FIELD( dims, (10, 10) );
}

Box3D ShapeFunctionXYZ::getBox(ProfileRT* profile) const
{
    Q_UNUSED(profile)
    if (m_bvh) return m_bvh->box();
    return Box3D();
}

bool ShapeFunctionXYZ::intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg, ProfileRT* profile) const
{  
    Q_UNUSED(profile)
    if (!m_bvh) return false;
    double tHitT = ray.tMax;
    DifferentialGeometry dgT;
    if (!m_bvh->intersect(ray, &tHitT, &dgT)) return false;

    if (tHit == 0 && dg == 0) return true;
    if (tHit == 0 || dg == 0) gcf::SevereError("ShapeMesh::intersect");

    *tHit = tHitT;
    *dg = dgT;
    dg->shape = this;
    return true;
}

void ShapeFunctionXYZ::updateShapeGL(TShapeKit* parent)
{
    // mesh
    buildMesh(parent);

    // visual
    SoShapeKit* shapeKit = parent->m_shapeKit;

    SoCoordinate3* sVertices = new SoCoordinate3;
    sVertices->point.setValues(0, vertices.size(), vertices.data());
    shapeKit->setPart("coordinate3", sVertices);

    SoNormal* sNormals = new SoNormal;
    sNormals->vector.setValues(0, normals.size(), normals.data());
    shapeKit->setPart("normal", sNormals);

    SoIndexedFaceSet* sMesh = new SoIndexedFaceSet;
    sMesh->coordIndex.setValues(0, faces.size(), faces.data());

    shapeKit->setPart("shape", sMesh);
}

ShapeFunctionXYZ::~ShapeFunctionXYZ()
{
    delete m_bvh;
    for (Triangle* t : m_triangles)
        delete t;
}

#include "kernel/scene/MaterialGL.h"
void ShapeFunctionXYZ::buildMesh(TShapeKit* parent)
{
    ProfileRT* profile = (ProfileRT*) parent->profileRT.getValue();
    QSize dimensions(dims.getValue()[0], dims.getValue()[1]);
    vec2d wh = profile->getBox().size();
    double resolutionU = wh.x/(dimensions.width() - 1);
    double resolutionV = wh.y/(dimensions.height() - 1);
    double resolution = std::min(resolutionU, resolutionV);

    MaterialGL* mGL = (MaterialGL*) parent->material.getValue();
    bool reverseNormals = mGL->reverseNormals.getValue();

    vertices.clear();
    normals.clear();
    faces.clear();

    if (ProfilePolygon* profilePolygon = dynamic_cast<ProfilePolygon*>(profile))
    {
        const QPolygonF& qpolygon = profilePolygon->getPolygon();

        QPointF prev = qpolygon[qpolygon.size() - 1];
        for (int n = 0; n < qpolygon.size(); ++n) {
            QPointF ed = qpolygon[n] - prev;
            prev = qpolygon[n];
            double se = 0.3*sqrt(ed.x()*ed.x() + ed.y()*ed.y());
            if (se < resolution) resolution = se;
        }

//        qDebug() << "Start polygon meshing";
        PolygonMesh polygonMesh(qpolygon);
        polygonMesh.makeMesh(resolution);
//        qDebug() << "Finished polygon meshing";

        for (const vec2d& uv : polygonMesh.getPoints()) {
            vertices << SbVec3f(uv.x, uv.y, 0.);
        }

        for (const auto& tri : polygonMesh.getTriangles()) {
            faces << tri.a << tri.b << tri.c << -1;
        }
    }
    else
    {
        QVector<vec2d> uvs = profile->makeMesh(dimensions);

        for (const vec2d& uv : uvs) {
            vertices << SbVec3f(uv.x, uv.y, 0.);
        }

        for (int n = 0; n < dimensions.width() - 1; ++n)
            for (int m = 0; m < dimensions.height() - 1; ++m) {
                int iA = n * dimensions.height() + m;
                int iB = iA + dimensions.height();
                int iC = iB + 1;
                int iD = iA + 1;
                faces << iA << iB << iC << -1;
                faces << iA << iC << iD << -1;
            }
    }

    // fill z
    QJSEngine engine;
    QString textX = functionX.getValue().getString();
    QString textY = functionY.getValue().getString();
    QString textZ = functionZ.getValue().getString();
    QJSValue object = engine.evaluate(QString(
        "({ unitName: 'Shape', "
        "fX: function(u, v) {return %1;}, "
        "fY: function(u, v) {return %2;}, "
        "fZ: function(u, v) {return %3;} "
        " })"
    ).arg(textX, textY, textZ));
    QJSValue fX = object.property("fX");
    QJSValue fY = object.property("fY");
    QJSValue fZ = object.property("fZ");

    normals = vertices;
    for (SbVec3f& p : vertices) {
        QJSValue ansX = fX.call(QJSValueList() << p[0] << p[1]);
        QJSValue ansY = fY.call(QJSValueList() << p[0] << p[1]);
        QJSValue ansZ = fZ.call(QJSValueList() << p[0] << p[1]);
        p.setValue(ansX.toNumber(), ansY.toNumber(), ansZ.toNumber());
    }

    double hu = resolutionU/100;
    double hv = resolutionV/100;
    for (SbVec3f& p : normals) {
        double u0 = p[0];
        double v0 = p[1];
        double pL, pR;

        vec3d dfdu;
        pL = fX.call(QJSValueList() << u0 - hu << v0).toNumber();
        pR = fX.call(QJSValueList() << u0 + hu << v0).toNumber();
        dfdu.x = (pR - pL)/(2*hu);
        pL = fY.call(QJSValueList() << u0 - hu << v0).toNumber();
        pR = fY.call(QJSValueList() << u0 + hu << v0).toNumber();
        dfdu.y = (pR - pL)/(2*hu);
        pL = fZ.call(QJSValueList() << u0 - hu << v0).toNumber();
        pR = fZ.call(QJSValueList() << u0 + hu << v0).toNumber();
        dfdu.z = (pR - pL)/(2*hu);

        vec3d dfdv;
        pL = fX.call(QJSValueList() << u0 << v0 - hv).toNumber();
        pR = fX.call(QJSValueList() << u0 << v0 + hv).toNumber();
        dfdv.x = (pR - pL)/(2*hv);
        pL = fY.call(QJSValueList() << u0 << v0 - hv).toNumber();
        pR = fY.call(QJSValueList() << u0 << v0 + hv).toNumber();
        dfdv.y = (pR - pL)/(2*hv);
        pL = fZ.call(QJSValueList() << u0 << v0 - hv).toNumber();
        pR = fZ.call(QJSValueList() << u0 << v0 + hv).toNumber();
        dfdv.z = (pR - pL)/(2*hv);

        vec3d nv = cross(dfdu, dfdv);
        nv.normalize();
        if (reverseNormals) nv = -nv;
        p.setValue(nv.x, nv.y, nv.z);
    }

    // fill triangles

    for (Triangle* t : m_triangles)
        delete t;
    m_triangles.clear();

    for (int n = 0; n < faces.size(); n += 4)
    {
        int iA = faces[n];
        int iB = faces[n + 1];
        int iC = faces[n + 2];
        Triangle* triangle = new Triangle(
            &vertices[iA][0], &vertices[iB][0], &vertices[iC][0],
            &normals[iA][0], &normals[iB][0], &normals[iC][0]);
        m_triangles.push_back(triangle);
    }

    if (m_bvh) delete m_bvh;
    m_bvh = new BVH(&m_triangles);
}
