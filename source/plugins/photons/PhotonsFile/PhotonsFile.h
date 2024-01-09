#pragma once

#include <QMap>
#include <QString>

#include "kernel/photons/PhotonsAbstract.h"

class Photon;

class PhotonsFile: public PhotonsAbstract
{

public:
    PhotonsFile();

    static QStringList getParameterNames() {return {"ExportDirectory", "ExportFile", "FileSize"};}
    void setParameter(QString name, QString value);

    bool startExport();
    void savePhotons(const std::vector<Photon>& photons);
    void setPhotonPower(double p) {m_photonPower = p;}
    void endExport();

    NAME_ICON_FUNCTIONS("File", ":/PhotonsFile.png")

private:
    void writePhotons(QString fileName, const std::vector<Photon>& photon, ulong nBegin, ulong nEnd);

    QString m_dirName;
    QString m_fileName;
    bool m_oneFile;
    ulong m_nPhotonsPerFile;

    int m_fileCurrent;
    ulong m_exportedPhotons;
    double m_photonPower;
    QVector<InstanceNode*> m_surfaces; //? map
	QVector<Transform> m_surfaceWorldToObject;
};



#include "PhotonsFileWidget.h"

class PhotonsFileFactory:
    public QObject,
    public PhotonsFactoryT<PhotonsFile, PhotonsFileWidget>
{
    Q_OBJECT
    Q_INTERFACES(PhotonsFactory)
    Q_PLUGIN_METADATA(IID "tonatiuh.PhotonsFactory")
};
