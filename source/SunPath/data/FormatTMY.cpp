#include "SunPath/data/FormatTMY.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QString>

using Qt::endl;

namespace sp {



FormatTMY::FormatTMY(SunTemporal* sunTemporal):
    m_sunTemporal(sunTemporal)
{

}

bool FormatTMY::read(QString fileName, const ParamsTMY& params)
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
        m_message = QString("Runtime error in FormatTMY::read\n") + message;
        qDebug() << m_message;
        return false;
    }
}

bool FormatTMY::write(QString fileName, const ParamsTMY& params)
{
    try {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            throw QString("File not opened: ") + fileName;

        QTextStream fout(&file);

        SunCalculator* sc = m_sunTemporal->calculator();
        fout << QString("Source,Latitude,Longitude,Time Zone\n");
        fout << QString("TMY3,%1,%2,%3\n")
                .arg(sc->location().latitude()/sp::degree, 0, 'f', 6)
                .arg(sc->location().longitude()/sp::degree, 0, 'f', 6)
                .arg(sc->location().offsetUTC()/3600., 0, 'f', 3);

        QString temp("Year,Month,Day,Hour,Minute,");
        if (params.seconds) temp += "Seconds,";
        temp += "DNI[W/m2]\n";
        fout << temp;

        const QVector<TimeStamp>& timeStampes = m_sunTemporal->timeStamps();
        const QVector<double>& data = m_sunTemporal->data();
        for (int n = 1; n < timeStampes.size(); ++n) {
            QDateTime dt = timeStampes[n].t;
            if (params.offset != 0)
                dt = dt.addSecs(params.offset);

            QString temp;

            QDate d = dt.date();
            temp += QString::number(d.year()) + ",";
            temp += QString::number(d.month()) + ",";
            temp += QString::number(d.day()) + ",";

            QTime t = dt.time();
            temp += QString::number(t.hour()) + ",";
            temp += QString::number(t.minute()) + ",";
            if (params.seconds) temp += QString::number(t.second()) + ",";

            temp += QString::number(data[n - 1],'f', params.precision) + "\n";

            fout << temp;
        }

        m_message.clear();
        return true;
    }
    catch (const QString& message) {
        m_message = QString("Runtime error in FormatTMY::write\n") + message;
        qDebug() << m_message;
        return false;
    }
}

void FormatTMY::readInfo(QTextStream& fin, const ParamsTMY& /*params*/)
{
    // line 1
    QString line = fin.readLine();
    if (line.isNull())
        throw QString("line 1 is missing");
    QStringList list = line.split(',');

    int iLatitude = -1;
    int iLongitude = -1;
    int iTimeZone = -1;
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].contains("Latitude", Qt::CaseInsensitive))
            iLatitude = i;
        else if (list[i].contains("Longitude", Qt::CaseInsensitive))
            iLongitude = i;
        else if (list[i].contains("Time Zone", Qt::CaseInsensitive))
            iTimeZone = i;
    }

    // line 2
    line = fin.readLine();
    if (line.isNull())
        throw QString("line 2 is missing");
    list = line.split(',');
    bool ok;

    double latitude = 0.;
    if (iLatitude > 0) {
        latitude = list[iLatitude].toDouble(&ok);
        if (!ok) throw QString("toDouble ") + line;
    }
    double longitude = 0.;
    if (iLongitude > 0) {
        longitude = list[iLongitude].toDouble(&ok);
        if (!ok) throw QString("toDouble ") + line;
    }

    int offsetFromUTC = 0;
    if (iTimeZone > 0) {
        offsetFromUTC = list[iTimeZone].toDouble(&ok)*3600;
        if (!ok) throw QString("toDouble ") + line;
    }
    Location location("TMY", latitude*sp::degree, longitude*sp::degree, offsetFromUTC);
    m_sunTemporal->calculator()->setLocation(location);
}

void FormatTMY::readData(QTextStream& fin, const ParamsTMY& params)
{
    // line 3
    QString line = fin.readLine();
    if (line.isNull()) throw QString("line 1 is missing");
    QStringList list = line.split(',');

    int iYear = -1;
    int iMonth = -1;
    int iDay = -1;
    int iHour = -1;
    int iMinute = -1;
    int iSecond = -1;
    int iDNI = -1;

    for (int i = 0; i < list.size(); ++i) {
        if (list[i].contains("Year", Qt::CaseInsensitive))
            iYear = i;
        else if (list[i].contains("Month", Qt::CaseInsensitive))
            iMonth = i;
        else if (list[i].contains("Day", Qt::CaseInsensitive))
            iDay = i;
        else if (list[i].contains("Hour", Qt::CaseInsensitive))
            iHour = i;
        else if (list[i].contains("Minute", Qt::CaseInsensitive))
            iMinute = i;
        else if (list[i].contains("Second", Qt::CaseInsensitive))
            iSecond = i;
        else if (list[i].contains("DNI", Qt::CaseInsensitive))
            iDNI = i;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    double DNI = 0.;

    bool ok;
    int offsetUTC = m_sunTemporal->calculator()->location().offsetUTC();
    QVector<QDateTime> ts;
    ts << QDateTime();
    QVector<double> ds;

    while (true) {
        QString line = fin.readLine();
        if (line.isNull()) break;
        QStringList list = line.split(',');

        year = list[iYear].toInt(&ok);
        if (!ok) throw QString("toInt ") + line;

        month = list[iMonth].toInt(&ok);
        if (!ok) throw QString("toInt ") + line;

        day = list[iDay].toInt(&ok);
        if (!ok) throw QString("toInt ") + line;

        hour = list[iHour].toInt(&ok);
        if (!ok) throw QString("toInt ") + line;

        if (iMinute > 0) {
            minute = list[iMinute].toInt(&ok);
            if (!ok) throw QString("toInt ") + line;
        }

        if (iSecond > 0) {
            second = list[iSecond].toInt(&ok);
            if (!ok) throw QString("toInt ") + line;
        }

        DNI = list[iDNI].toDouble(&ok);
        if (!ok) throw QString("toDouble ") + line;

        ts << QDateTime(QDate(year, month, day), QTime(hour, minute, second), Qt::OffsetFromUTC, offsetUTC);
        ds << DNI;
    }

    int tStep = ts[1].msecsTo(ts[2]);
    ts[0] = ts[1].addMSecs(-tStep);

    if (params.offset != 0)
        for (QDateTime& dt : ts)
            dt = dt.addSecs(params.offset);

    TimeSampler timeSampler(m_sunTemporal);
    timeSampler.sample(ts, params.adjustDay);

    m_sunTemporal->setData(ds);
}


} // namespace sp
