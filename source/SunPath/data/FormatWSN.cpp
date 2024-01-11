#include "FormatWSN.h"

#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>

using Qt::endl;

namespace sp {


//  QDate::currentDate().toString("yyyy-MM-dd")

FormatWSN::FormatWSN(SunSpatial* sunSpatial):
    m_sunSpatial(sunSpatial)
{

}

bool FormatWSN::read(QString fileName, const ParamsWSN& params)
{
    try {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            throw QString("File not opened: ") + fileName;

        QTextStream fin(&file);
        readInfo(fin, params);
        readData(fin, params);

        m_message.clear();
        return true;
    }
    catch (const QString& message) {
        m_message = QString("Runtime error in FormatWSN::read\n") + message;
        qDebug() << m_message;
        return false;
    }
}

bool FormatWSN::write(QString fileName, const ParamsWSN& params)
{
    try {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            throw QString("File not opened: ") + fileName;

        QXmlStreamWriter xml(&file);
//        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.device()->write("\n");
        xml.writeStartElement("SunPath");
        xml.writeAttribute("version", "2021.4");
        xml.writeCharacters("");
        xml.device()->write("\n");

        xml.writeStartElement("Header");
        xml.writeCharacters(writeInfo(params));
        xml.writeEndElement();
        xml.device()->write("\n");

        xml.writeTextElement("Data", writeData(params));
        xml.device()->write("\n");

        xml.writeEndElement();
        xml.writeEndDocument();

        m_message.clear();
        return true;
    }
    catch (const QString& message) {
        m_message = QString("Runtime error in FormatWSN::write\n") + message;
        qDebug() << m_message;
        return false;
    }
}

void FormatWSN::readInfo(QTextStream& /*fin*/, const ParamsWSN& /*params*/)
{
//    QString line = fin.readLine();
//    if (line != "Sampling of sun path")
//        throw QString("line 1 should be \"Sampling of sun path\"");

//    line = fin.readLine();
//    if (!line.isEmpty())
//        throw QString("line 2 should be empty");

//    line = fin.readLine();
//    QStringList list = line.split(' ');
//    if (line != "Name:")
//        throw QString("line 3 should start with \"Name:\"");
//    QString name = list[1];

//    line = fin.readLine();
//    list = line.split(' ');
//    if (line != "Latitude:")
//        throw QString("line 4 should start with \"Latitude:\"");
//    double latitude = list[1].toDouble()*degree;

//    line = fin.readLine();
//    list = line.split(' ');
//    if (line != "Longitude:")
//        throw QString("line 5 should start with \"Latitude:\"");
//    double longitude = list[1].toDouble()*degree;

//    line = fin.readLine();

//    Location location(name, latitude*degree, longitude*degree);
//    m_sunSpatial->calculator()->setLocation(location);
//    m_sunSpatial->setInfo();
}

void FormatWSN::readData(QTextStream& /*fin*/, const ParamsWSN& /*params*/)
{

}

QString FormatWSN::writeInfo(const ParamsWSN& /*params*/)
{
    QString ans;
    QTextStream fout(&ans);
    fout << endl;

    SunCalculator* sun = m_sunSpatial->calculator();
    fout << QString("Name: %1\n").arg(sun->location().name());
    fout << QString("Latitude: %1 deg\n").arg(sun->location().latitude()/degree, 0, 'f', 4);
    fout << QString("Longitude: %1 deg\n").arg(sun->location().longitude()/degree, 0, 'f', 4);
    fout << endl;

    int nMax = m_sunSpatial->skyNodes().size();
    fout << QString("Nodes: %1\n").arg(nMax);

    if (!m_sunSpatial->info().isEmpty())
        fout << m_sunSpatial->info() << endl;

    return ans;
}

QString FormatWSN::writeData(const ParamsWSN& params)
{
    QString ans;
    QTextStream fout(&ans);
    fout << endl;

    QString temp;
    int cw = 16;

    temp += QString("Index").rightJustified(cw) + ",";
    temp += QString("Azimuth [deg]").rightJustified(cw) + ",";
    temp += QString("Elevation [deg]").rightJustified(cw) + ",";
    temp += QString("Sigma [deg]").rightJustified(cw) + ",";

    temp += QString("Function []").rightJustified(cw) + ",";
    if (params.withAmplitudes)
        temp += QString("Amplitude []").rightJustified(cw) + ",";
    temp += QString("Weight [Wh/m2]").rightJustified(cw) + ",";
    temp += QString("Weight [%]").rightJustified(cw);

    fout << temp << endl;

    SunCalculator* sun = m_sunSpatial->calculator();
    const QVector<SkyNode>& skyNodes = m_sunSpatial->skyNodes();
    const QVector<double>& values = m_sunSpatial->values();
    const QVector<double>& weights = m_sunSpatial->weights();
    const QVector<double>& amplitudes = m_sunSpatial->amplitudes();

    double wTotal = 0.;
    for (double w : weights) wTotal += w;

    for (int n = 0; n < skyNodes.size(); ++n) {
        QString temp;

        const SkyNode& sn = skyNodes[n];
        Horizontal hc = sun->findHorizontal(sn.v);
        if (params.positiveAzimuth) hc.makeAzimuthPositive();

        temp += QString::number(n + 1).rightJustified(cw) + ",";
        temp += QString::number(hc.azimuth()/degree, 'f', 4).rightJustified(cw) + ",";
        temp += QString::number(hc.elevation()/degree, 'f', 4).rightJustified(cw) + ",";
        temp += QString::number(sn.sigma/degree, 'f', 4).rightJustified(cw) + ",";

        double v = values[n];
        double w = weights[n];

        temp += QString::number(v, 'f', params.precision).rightJustified(cw) + ",";
        if (params.withAmplitudes) {
            double a = amplitudes[n];
            temp += QString::number(a, 'f', params.precision).rightJustified(cw) + ",";
        }
        temp += QString::number(w, 'f', 3).rightJustified(cw) + ",";
        temp += QString::number(w/wTotal*100, 'f', 6).rightJustified(cw);

        fout << temp << endl;
    }

    return ans;
}


} // namespace sp
