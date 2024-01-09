#include "ShapeFunctionZ.h"

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

SO_NODE_SOURCE(ShapeFunctionZ)


void ShapeFunctionZ::initClass()
{
    SO_NODE_INIT_CLASS(ShapeFunctionZ, ShapeRT, "ShapeRT");
}

ShapeFunctionZ::ShapeFunctionZ()
{  
    m_bvh = 0;
    SO_NODE_CONSTRUCTOR(ShapeFunctionZ);

    SO_NODE_ADD_FIELD( functionZ, ("(x*x + y*y)/4") );
    SO_NODE_ADD_FIELD( dims, (10, 10) );
}

Box3D ShapeFunctionZ::getBox(ProfileRT* profile) const
{
    Q_UNUSED(profile)
    if (m_bvh) return m_bvh->box();
    return Box3D();
}

bool ShapeFunctionZ::intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg, ProfileRT* profile) const
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

void ShapeFunctionZ::updateShapeGL(TShapeKit* parent)
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

ShapeFunctionZ::~ShapeFunctionZ()
{
    delete m_bvh;
    for (Triangle* t : m_triangles)
        delete t;
}

#include "kernel/scene/MaterialGL.h"
void ShapeFunctionZ::buildMesh(TShapeKit* parent)
{
    ProfileRT* profile = (ProfileRT*) parent->profileRT.getValue();
    QSize dimensions(dims.getValue()[0], dims.getValue()[1]);
    vec2d wh = profile->getBox().size();
    double resolution = std::min(wh.x/(dimensions.width() - 1), wh.y/(dimensions.height() - 1));

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
    QString fZ = functionZ.getValue().getString();
    QJSValue object = engine.evaluate(QString(
        "({ unitName: 'Shape', "
        "findZ: function(x, y) {return %1;}"
        " })"
    ).arg(fZ));
    QJSValue findZ = object.property("findZ");


    for (SbVec3f& v : vertices) {
        QJSValue ansZ = findZ.call(QJSValueList() << v[0] << v[1]);
        v[2] = ansZ.toNumber();
    }

    normals = vertices;
    double h = resolution/100;
    for (SbVec3f& v : normals) {
        double x0 = v[0];
        double y0 = v[1];

        double zL = findZ.call(QJSValueList() << x0 - h << y0).toNumber();
        double zR = findZ.call(QJSValueList() << x0 + h << y0).toNumber();
        double dfx = (zR - zL)/(2*h);

        zL = findZ.call(QJSValueList() << x0 << y0 - h).toNumber();
        zR = findZ.call(QJSValueList() << x0 << y0 + h).toNumber();
        double dfy = (zR - zL)/(2*h);

        vec3d nv(-dfx, -dfy, 1);
        nv.normalize();
        if (reverseNormals) nv = -nv;
        v.setValue(nv.x, nv.y, nv.z);
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
