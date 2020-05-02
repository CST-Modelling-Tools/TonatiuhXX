#include <QImage>
#include <QBrush>
#include <QColor>
#include <QImage>
#include <QPaintEngine>

#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoLabel.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodekits/SoNodekitCatalog.h>

#include "libraries/geometry/gc.h"
#include "libraries/geometry/BBox.h"
#include "libraries/geometry/Matrix4x4.h"
#include "libraries/geometry/Point3D.h"
#include "sun/sunpos.h"
#include "sun/SunPillbox.h"
#include "TLightKit.h"
#include "TLightShape.h"
#include "libraries/geometry/Transform.h"
#include "scene/TShapeKit.h"


struct Polygon
{
    Polygon() {}

    Polygon(QPointF p1, QPointF p2, QPointF p3, QPointF p4)
    {
        x[0] = p1.x();
        x[1] = p2.x();
        x[2] = p3.x();
        x[3] = p4.x();
        y[0] = p1.y();
        y[1] = p2.y();
        y[2] = p3.y();
        y[3] = p4.y();
    }

    double x[4];
    double y[4];
};

bool PixelInPolygon(int x, int y, Polygon p)
{
    bool c = false;
    int nVert = 4;
    for (int i = 0, j = nVert - 1; i < nVert; j = i++)
    {
        /*if( ( ( ( p.y[i] <= y ) && ( y < p.y[j] ) ) || ( ( p.y[j] <= y ) && ( y < p.y[i] ) ) ) &&
                                ( x < ( p.x[j] - p.x[i] ) * ( y - p.y[i] ) / ( p.y[j] -p.y[i] ) + p.x[i] ) )
                        c = !c;
                 */
        if ( ( (p.y[i] > y) != (p.y[j] > y) ) &&
             (x < (p.x[j] - p.x[i]) * (y - p.y[i]) / (p.y[j] - p.y[i]) + p.x[i]) )
            c = !c;
    }
    return c;
}


SO_KIT_SOURCE(TLightKit)

/**
 * Initializates TLightKit componets
 */
void TLightKit::initClass()
{
    SO_KIT_INIT_CLASS(TLightKit, SoLightKit, "LightKit");
}

/**
 * Creates a new TLightKit.
 */
TLightKit::TLightKit()
{
    SO_KIT_CONSTRUCTOR(TLightKit);

    SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(iconMaterial, SoNode, SoMaterial, TRUE, iconSeparator, icon, TRUE);
    SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(iconTexture, SoNode, SoTexture2, TRUE, iconSeparator, iconMaterial, TRUE);
    SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(tsunshape, SunAbstract, SunPillbox, TRUE, transformGroup, "", TRUE);

    SO_NODE_ADD_FIELD(azimuth, (0.) );
    SO_NODE_ADD_FIELD(zenith, (0.) );
    SO_NODE_ADD_FIELD(disabledNodes, ("") );


    SO_KIT_INIT_INSTANCE();

    SoDirectionalLight* light = static_cast<SoDirectionalLight*>(getPart("light", true) );
    light->direction.setValue(SbVec3f(0, -1, 0) );

    SoTransform* transform = new SoTransform;
    setPart("transform", transform);

    SoMaterial* lightMaterial = static_cast<SoMaterial*>(getPart("iconMaterial", true) );
    lightMaterial->transparency = 0.5;
    setPart("iconMaterial", lightMaterial);

    int widthPixeles = 10;
    int heightPixeles = 10;
    QVector<uchar> bitmap(widthPixeles * heightPixeles);
    bitmap.fill(255);

    SoTexture2* texture = new SoTexture2;
    texture->image.setValue(SbVec2s(heightPixeles, widthPixeles), 1, bitmap.data());
    texture->model = SoTexture2::BLEND;
    texture->blendColor.setValue(0.933f, 0.91f, 0.666f);
    setPart("iconTexture", texture);

    TLightShape* iconShape = new TLightShape;
    setPart("icon", iconShape);
}

/**
 * Destructor.
 */
TLightKit::~TLightKit()
{
    //void ChangePosition( QDateTime time, double longitude, double latitude );
    //void SetDateTime( QDateTime time );
}

/*!
 * Changes the light position to \a azimuth and \a zenith from the scene centre.
 * Azimuth and Zenith are in radians.
 * \sa redo().
 */
void TLightKit::ChangePosition(double newAzimuth, double newZenith)
{
    azimuth = newAzimuth;
    zenith = newZenith;
}

void TLightKit::Update(BBox box)
{
    double xMax = box.pMax.x;
    double xMin = box.pMin.x;
    double zMin = box.pMin.z;
    double zMax = box.pMax.z;


    double distMax = box.pMax.y + 10 - box.pMin.y;
    double back = box.pMax.y + 10;


    if (-gc::Infinity  == box.Volume() )
    {
        xMin = 0.0;
        xMax = 0.0;
        zMin = 0.0;
        zMax = 0.0;
        distMax = 0.0;
    }

    SunAbstract* sunshape = static_cast< SunAbstract* >(this->getPart("tsunshape", false) );
    if (!sunshape) return;
    double thetaMax = sunshape->getThetaMax();
    double delta = 0.01;
    if (thetaMax > 0.0) delta = distMax * tan(thetaMax);

    TLightShape* shape = static_cast< TLightShape* >(this->getPart("icon", false) );
    if (!shape) return;
    shape->xMin.setValue( (xMin - delta) );
    shape->xMax.setValue( (xMax + delta) );
    shape->zMin.setValue( (zMin - delta) );
    shape->zMax.setValue( (zMax + delta) );
    shape->delta.setValue(delta);


    SoTransform* lightTransform = static_cast< SoTransform* >(this->getPart("transform", false) );
    if (!lightTransform) return;
    SbVec3f translation(0.0, back, 0.0);
    lightTransform->translation.setValue(translation);

}

void TLightKit::ComputeLightSourceArea(int widthDivisions, int heigthDivisions, QVector< QPair< TShapeKit*, Transform > > surfacesList)
{
    TLightShape* shape = static_cast< TLightShape* >(this->getPart("icon", false) );
    if (!shape) return;

    double width =  shape->xMax.getValue() - shape->xMin.getValue();
    double height = shape->zMax.getValue() - shape->zMin.getValue();


    int widthPixeles = widthDivisions;
    while ( (width / widthPixeles) < shape->delta.getValue() ) widthPixeles--;
    double pixelWidth = double( width / widthPixeles );

    int heightPixeles = heigthDivisions;
    while ( (height / heightPixeles) < shape->delta.getValue() ) heightPixeles--;
    double pixelHeight = height / heightPixeles;


    QImage* sourceImage = new QImage(widthPixeles, heightPixeles, QImage::Format_RGB32);
    sourceImage->setOffset(QPoint(0.5, 0.5) );
    sourceImage->fill(Qt::white);

    QPainter painter(sourceImage);

    QBrush brush(Qt::black);
    painter.setBrush(brush);

    QPen pen(Qt::black);
    painter.setPen(pen);

    //painter.setRenderHint(   QPainter::Antialiasing);

    //QPen pen( Qt::black, Qt::NoPen );
    //painter.setPen( pen );

    for (int s = 0; s < surfacesList.size(); s++)
    {
        TShapeKit* surfaceKit = surfacesList[s].first;
        Transform surfaceTransform = surfacesList[s].second;
        Transform shapeToWorld = surfaceTransform.GetInverse();

        ShapeAbstract* shapeNode = static_cast< ShapeAbstract* > (surfaceKit->getPart("shape", false) );
        if (shapeNode)
        {
            BBox shapeBB = shapeNode->getBox();

            Point3D p1(shapeBB.pMin.x, shapeBB.pMin.y, shapeBB.pMin.z);
            Point3D p2(shapeBB.pMax.x, shapeBB.pMin.y, shapeBB.pMin.z);
            Point3D p3(shapeBB.pMax.x, shapeBB.pMin.y, shapeBB.pMax.z);
            Point3D p4(shapeBB.pMin.x, shapeBB.pMin.y, shapeBB.pMax.z);
            Point3D p5(shapeBB.pMin.x, shapeBB.pMax.y, shapeBB.pMin.z);
            Point3D p6(shapeBB.pMax.x, shapeBB.pMax.y, shapeBB.pMin.z);
            Point3D p7(shapeBB.pMax.x, shapeBB.pMax.y, shapeBB.pMax.z);
            Point3D p8(shapeBB.pMin.x, shapeBB.pMax.y, shapeBB.pMax.z);


            Point3D tP1 = shapeToWorld(p1);
            Point3D tP2 = shapeToWorld(p2);
            Point3D tP3 = shapeToWorld(p3);
            Point3D tP4 = shapeToWorld(p4);
            Point3D tP5 = shapeToWorld(p5);
            Point3D tP6 = shapeToWorld(p6);
            Point3D tP7 = shapeToWorld(p7);
            Point3D tP8 = shapeToWorld(p8);

            QPoint qP1( (tP1.x - shape->xMin.getValue() ) / pixelWidth, (tP1.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP2( (tP2.x - shape->xMin.getValue() ) / pixelWidth, (tP2.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP3( (tP3.x - shape->xMin.getValue() ) / pixelWidth, (tP3.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP4( (tP4.x - shape->xMin.getValue() ) / pixelWidth, (tP4.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP5( (tP5.x - shape->xMin.getValue() ) / pixelWidth, (tP5.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP6( (tP6.x - shape->xMin.getValue() ) / pixelWidth, (tP6.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP7( (tP7.x - shape->xMin.getValue() ) / pixelWidth, (tP7.z - shape->zMin.getValue() ) / pixelHeight);
            QPoint qP8( (tP8.x - shape->xMin.getValue() ) / pixelWidth, (tP8.z - shape->zMin.getValue() ) / pixelHeight);

            QPointF polygon1[4] = { qP1, qP2, qP3, qP4 };
            QPointF polygon2[4] = { qP1, qP2, qP6, qP5 };
            QPointF polygon3[4] = { qP1, qP4, qP8, qP5 };
            QPointF polygon4[4] = { qP2, qP3, qP7, qP6 };
            QPointF polygon5[4] = { qP3, qP4, qP8, qP7 };
            QPointF polygon6[4] = { qP5, qP6, qP7, qP8 };


            painter.drawPoint(qP1);
            painter.drawPoint(qP2);
            painter.drawPoint(qP3);
            painter.drawPoint(qP4);
            painter.drawPoint(qP5);
            painter.drawPoint(qP6);
            painter.drawPoint(qP7);
            painter.drawPoint(qP8);


            painter.drawPolygon(polygon1, 4);
            painter.drawPolygon(polygon2, 4);
            painter.drawPolygon(polygon3, 4);
            painter.drawPolygon(polygon4, 4);
            painter.drawPolygon(polygon5, 4);
            painter.drawPolygon(polygon6, 4);
        }

    }

    int** areaMatrix = new int*[heightPixeles];
    for (int i = 0; i < heightPixeles; i++)
    {
        areaMatrix[i] = new int[widthPixeles];
    }


    //unsigned char bitmap[ widthPixeles * heightPixeles ];
    unsigned char* bitmap = new unsigned char[ widthPixeles * heightPixeles ];

    QRgb black = qRgb(0, 0, 0);

    for (int i = 0; i < widthPixeles; i++)
    {
        for (int j = 0; j < heightPixeles; j++)
        {

            double pixelIntensity = (sourceImage->pixel(i, j) == black) ? 1 : 0;
            if ( (i - 1) >= 0 && (j - 1) >= 0) pixelIntensity += (sourceImage->pixel(i - 1, j - 1) == black) ? 1 : 0;
            if ( (j - 1) >= 0) pixelIntensity += (sourceImage->pixel(i, j - 1) == black) ? 1 : 0;
            if ( (i + 1) < widthPixeles && (j - 1) >= 0) pixelIntensity += (sourceImage->pixel(i + 1, j - 1) == black) ? 1 : 0;
            if ( (i - 1) >= 0) pixelIntensity += (sourceImage->pixel(i - 1, j) == black) ? 1 : 0;
            if ( (i + 1) < widthPixeles) pixelIntensity += (sourceImage->pixel(i + 1, j) == black) ? 1 : 0;
            if ( (i - 1) >= 0 && (j + 1) < heightPixeles) pixelIntensity += (sourceImage->pixel(i - 1, j + 1) == black) ? 1 : 0;
            if ( (j + 1) < heightPixeles) pixelIntensity += (sourceImage->pixel(i, j + 1) == black) ? 1 : 0;
            if ( (i + 1) < widthPixeles && (j + 1) < heightPixeles) pixelIntensity += (sourceImage->pixel(i + 1, j + 1) == black) ? 1 : 0;

            if (pixelIntensity > 0.0)
            {
                areaMatrix[j][i] = 1;
                bitmap[ i * heightPixeles +  j ] = 0;
            }
            else
            {
                areaMatrix[j][i] = 0;
                bitmap[ i * heightPixeles +  j ] = 255;
            }

        }
    }


//    SoTexture2* texture = static_cast< SoTexture2* >(getPart("iconTexture", true) );
//    texture->image.setValue(SbVec2s(heightPixeles, widthPixeles), 1, bitmap);
//    texture->wrapS = SoTexture2::CLAMP;
//    texture->wrapT = SoTexture2::CLAMP;

    delete[] bitmap;

    shape->SetLightSourceArea(heightPixeles, widthPixeles, areaMatrix);
}
