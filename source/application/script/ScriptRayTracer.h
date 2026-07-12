#pragma once

#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>
#include <QVector>

class Document;
class GraphicRoot;
class InstanceNode;
class QScriptContext;
class SceneTreeModel;
class PhotonsBuffer;
class Transform;


class ScriptRayTracer: public QObject
{
    Q_OBJECT

public:
    ScriptRayTracer();
    ~ScriptRayTracer();

    void Clear();

    QString getDir() {return m_dirName;}

    bool IsValidSurface(QString surfaceName);

    double GetArea() {return m_area;}
    double GetNumrays() {return m_numberOfRays;}

    int setDir(QString dir);

    int setIrradiance(double irradiance);

    int setNumberOfRays(double nrays);

    int SetNumberOfWidthDivisions(int wdivisions);
    int SetNumberOfHeightDivisions(int hdivisions);

    int SetPhotonMapExportMode(QString typeName);


    void SetSunAzimtuh(double azimuth);
    void SetSunElevation(double elevation);

    void SetupGraphcisRoot();
    void SetupModels();
    int openFile(QString filename);

    int Trace();

    int SetSunPositionToScene();
    int Save(const QString& fileName);

private:
    Document* m_document;

    double m_irradiance;

    ulong m_numberOfRays;

    PhotonsBuffer* m_photonMap;
    bool m_photonMapToFile;

    SceneTreeModel* m_sceneModel;

    double m_area;
    int m_widthDivisions;
    int m_heightDivisions;
    bool m_sunPosistionChanged;
    double m_sunAzimuth;
    double m_sunElevation;

    double m_wPhoton;

    QString m_dirName;
};
