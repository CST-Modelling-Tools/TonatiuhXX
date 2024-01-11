#include "ReaderTMY.h"

#include <QFile>
#include <QStringList>

#include "ParameterItem.h"
#include "SunPath/calculators/SunCalculatorMB.h"
#include "../DaysEdit/DaysModel.h"

using Qt::endl;

namespace sp {


int readInt(const QStringList& list, int n)
{
    if (n < 0) return 0;
    bool ok;
    int ans = list[n].toInt(&ok);
    if (!ok) throw QString("toInt at %1 of \n").arg(n) + list.join(',');
    return ans;
}

double readDouble(const QStringList& list, int n)
{
    if (n < 0) return 0.;
    bool ok;
    double ans = list[n].toDouble(&ok);
    if (!ok) throw QString("toDouble at %1 of \n").arg(n) + list.join(',');
    return ans;
}

bool comparator(const RecordTMY& r, const QDate& date)
{
//    if (2001 < date.year())
//        return true;
//    if (2001 > date.year())
//        return false;

    if (r.time.date().month() < date.month())
        return true;
    if (r.time.date().month() > date.month())
        return false;

    if (r.time.date().day() < date.day())
        return true;

    return false;
}


bool comparatorStamp(const RecordTMY& r, int stamp)
{
    return r.stamp < stamp;
}

ReaderTMY::ReaderTMY()
{
    m_paramsOriginal = 0;
    m_paramsEffective = 0;
}

ReaderTMY::~ReaderTMY()
{

}

void ReaderTMY::setModels(ParameterModel* paramsOriginal, ParameterModel* paramsEffective)
{
    m_paramsOriginal = paramsOriginal;
    m_paramsEffective = paramsEffective;
}

bool ReaderTMY::read(QString fileName)
{
    try {
        m_message.clear();

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            throw QString("File not opened: ") + fileName;

        QTextStream fin(&file);
        readInfo(fin);
        readData(fin);
        return true;
    }
    catch (const QString& message) {
        m_message = QString("Runtime error in FormatTMY::read\n") + message;
        qDebug() << m_message;
        return false;
    }
}


int ReaderTMY::findIndex(QDate date)
{
    auto iter = std::lower_bound(m_data.begin(), m_data.end(), date, comparator);
    int ans = iter - m_data.begin();
    return ans;
}

int ReaderTMY::findIndexStamp(int stamp)
{
    auto iter = std::lower_bound(m_data.begin(), m_data.end(), stamp, comparatorStamp);
    int ans = iter - m_data.begin();
    return ans;
}

int ReaderTMY::findStamp(QDate date)
{
    DaysModel dm;
    bool leap =  m_paramsEffective->parameterDouble("Checks.Time in February")->value() > 28*24*3600;
    dm.setLeap(leap);
    dm.setMonthDay(date.month(), date.day());
    int ans = (dm.dayNumber() - 1)*24*3600;
    return ans;
}

void ReaderTMY::setDNI(QString name)
{
    int n = m_recordsInfo.names.indexOf(name);
    if (n < 0) return;

    bool ok;
        for (RecordTMY& r : m_data) {
        double dni = r.data.section(',', n, n).toDouble(&ok);
//        if (!ok) throw QString("toDouble ") + line;
        r.DNI = dni;
    }

    m_recordsInfo.min = std::min_element(m_data.begin(), m_data.end(),
        [](const auto& a, const auto& b) {return a.DNI < b.DNI;}
    )->DNI;
    m_recordsInfo.max = std::max_element(m_data.begin(), m_data.end(),
        [](const auto& a, const auto& b) {return a.DNI < b.DNI;}
    )->DNI;

    int tStep = m_data[0].time.secsTo(m_data[1].time);
    m_paramsEffective->parameterDouble("Time.Step")->setValue(tStep);

    QDateTime& dt0 = m_data[0].time;
    QDateTime dty(QDate(dt0.date().year(), 1, 1), QTime(0, 0), Qt::UTC);
    int stamp = dty.secsTo(dt0);

    for (RecordTMY& r : m_data) {
        r.stamp = stamp;
        stamp += tStep;
    }

    int tTotal = m_data.size()*tStep;
    m_paramsEffective->parameterDouble("Checks.Time total")->setValue(tTotal);

    int a = findIndex(QDate(2001, 2, 1));
    int b = findIndex(QDate(2001, 3, 1));
    int tTotalF = (b - a)*tStep;
    m_paramsEffective->parameterDouble("Checks.Time in February")->setValue(tTotalF);
//    m_paramsEffective->parameterDouble("Checks.Hour 0 as 24")->setValue(0.); //todo n23, n0 or n23 (n+1)0

    double dniTotal = 0.;
    for (const RecordTMY& r : m_data) {
        dniTotal += r.DNI;
    }
    dniTotal *= tStep;
    m_paramsEffective->parameterDouble("Checks.DNI total")->setValue(dniTotal);

    double days = tTotal/(3600*24);
    double dniDaily = dniTotal/days;
    m_paramsEffective->parameterDouble("Checks.DNI daily")->setValue(dniDaily);

    // fix0h
    if (m_paramsEffective->parameterDouble("Time.Fix 0h")->value() > 0)
    {
//        for (int n = 0; n < m_data.size(); n++) {
//            RecordTMY
//        }

    }

    // dni below horizon
    double origin = m_paramsEffective->parameterDouble("Time.Origin")->value();
    double dniBelow = calculateDNIbelow(origin);
    m_paramsEffective->parameterDouble("Checks.DNI below")->setValue(dniBelow);

    bool hoursAscending = isTimeSorted();
    m_paramsEffective->parameterAbstract("Checks.Hour 0 as 24")->setText(hoursAscending? "false" : "true");
}

void ReaderTMY::readInfo(QTextStream& fin)
{
    // line 1
    QString line = fin.readLine();
    if (line.isNull())
        throw QString("line 1 is missing");
    m_infoNames = line.split(',');

    // line 2
    line = fin.readLine();
    if (line.isNull())
        throw QString("line 2 is missing");
    m_info = line.split(',');

    if (m_infoNames.size() != m_info.size())
        throw QString("line 1 and 2 have a different number of elements");

    // extract
//    bool ok;
    int iLatitude = -1;
    int iLongitude = -1;
    int iTimeZone = -1;
    for (int i = 0; i < m_infoNames.size(); ++i) {
        if (m_infoNames[i].contains("Latitude", Qt::CaseInsensitive))
            iLatitude = i;
        else if (m_infoNames[i].contains("Longitude", Qt::CaseInsensitive))
            iLongitude = i;
        else if (m_infoNames[i].contains("Time Zone", Qt::CaseInsensitive))
            iTimeZone = i;
    }

    double latitude = readDouble(m_info, iLatitude);
    double longitude = readDouble(m_info, iLongitude);
    double timeZone = readDouble(m_info, iTimeZone);
    int offsetFromUTC = timeZone*3600; // can be negative, in seconds


    if (m_paramsOriginal) {
        m_paramsOriginal->cleanRows();
        for (int r = 0; r < m_infoNames.size(); r++) {
            QStandardItem* itemA = new QStandardItem(m_infoNames[r]);
            itemA->setEditable(false);
            QStandardItem* itemB = new QStandardItem(m_info[r]);
            itemB->setEditable(false);
            m_paramsOriginal->appendRow({itemA, itemB});
        }
    }

    if (m_paramsEffective) {
        m_paramsEffective->cleanRows();
        QStandardItem* itemRoot = m_paramsEffective->invisibleRootItem();
        QStandardItem* item;
        ParameterAbstract* param;

        item = m_paramsEffective->appendParameter(itemRoot, new ParameterAbstract("Location"));
        m_paramsEffective->appendParameter(item, new ParameterDouble("Latitude", latitude*sp::degree, "angle", "{.4f} deg"));
        m_paramsEffective->appendParameter(item, new ParameterDouble("Longitude", longitude*sp::degree, "angle", "{.4f} deg"));

        item = m_paramsEffective->appendParameter(itemRoot, new ParameterAbstract("Time"));

        param = new ParameterDouble("Zone", offsetFromUTC, "time", "{d}h {02d}m");
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("Step", 0., "time", "{d}h {02d}m");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("Origin", -1.0, "", "{.1f}");
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("Fix 0h", 0, "", "{.1f}");
        m_paramsEffective->appendParameter(item, param);

        item = m_paramsEffective->appendParameter(itemRoot, new ParameterAbstract("Calculator"));

        item = m_paramsEffective->appendParameter(itemRoot, new ParameterAbstract("Checks"));

        param = new ParameterDouble("Time total", 0., "time", "{d}d {02d}h {02d}m");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("Time in February", 0., "time", "{d}d {02d}h {02d}m");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterAbstract("Hour 0 as 24", "true");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("DNI total", 0., "energy", "{.3f} kWh");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("DNI daily", 0., "energy", "{.3f} kWh");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);

        param = new ParameterDouble("DNI below", 0., "energy", "{.3f} kWh");
        param->setEditable(false);
        m_paramsEffective->appendParameter(item, param);
    }
}

void ReaderTMY::readData(QTextStream& fin)
{
    // line 3
    QString line = fin.readLine();
    if (line.isNull())
        throw QString("line 3 is missing");

    QStringList& names = m_recordsInfo.names;
    names = line.split(',');

    int iYear = -1;
    int iMonth = -1;
    int iDay = -1;
    int iHour = -1;
    int iMinute = -1;
    int iSecond = -1;

    for (int i = 0; i < names.size(); ++i) {
        if (names[i].contains("Year", Qt::CaseInsensitive))
            iYear = i;
        else if (names[i].contains("Month", Qt::CaseInsensitive))
            iMonth = i;
        else if (names[i].contains("Day", Qt::CaseInsensitive))
            iDay = i;
        else if (names[i].contains("Hour", Qt::CaseInsensitive))
            iHour = i;
        else if (names[i].contains("Minute", Qt::CaseInsensitive))
            iMinute = i;
        else if (names[i].contains("Second", Qt::CaseInsensitive))
            iSecond = i;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    m_data.clear();
    RecordTMY record;
    while (true) {
        QString line = fin.readLine();
        if (line.isNull()) break;

        QStringList list = line.split(',');
        if (list.size() != names.size())
            throw QString("line 3 and N have a different number of elements");

        year = readInt(list, iYear);
        month = readInt(list, iMonth);
        day = readInt(list, iDay);
        hour = readInt(list, iHour);
        minute = readInt(list, iMinute);
        second = readInt(list, iSecond);

        record.time = QDateTime(
            QDate(year, month, day),
            QTime(hour, minute, second),
            Qt::UTC
        );
        record.data = line;
        m_data << record;
    }

    setDNI("DNI");
}

double ReaderTMY::calculateDNIbelow(double origin)
{
    SunCalculator* sunCalc = new SunCalculatorMB;
    double latitude = m_paramsEffective->parameterDouble("Location.Latitude")->value();
    double longitude = m_paramsEffective->parameterDouble("Location.Longitude")->value();
    int offsetUTC = m_paramsEffective->parameterDouble("Time.Zone")->value();
    int tStep = m_paramsEffective->parameterDouble("Time.Step")->value();

    int endTMY = origin*tStep + tStep;

    Location location("", latitude, longitude, offsetUTC);
    sunCalc->setLocation(location);

    double dniBelow = 0.;
    sp::Horizontal hcA = sunCalc->findHorizontalV(m_data[0].time.addSecs(-offsetUTC + endTMY - tStep));
    for (int n = 0; n < m_data.size(); n++) {
        sp::Horizontal hcB = sunCalc->findHorizontalV(m_data[n].time.addSecs(-offsetUTC + endTMY));
        if (hcA.elevation() < 0 && hcB.elevation() < 0.)
            dniBelow += m_data[n].DNI;
        hcA = hcB;
//        qDebug() << m_data[n].time.addSecs(-offsetUTC + offsetTMY) << hcB.elevation()/degree<< " " << m_data[n].DNI;
    }

    dniBelow *= tStep;
    delete sunCalc;
    return dniBelow;
}

bool ReaderTMY::isTimeSorted()
{
    int dPrev = 0;
    int hPrev = 0;
    for (int n = 0; n < m_data.size(); n++) {
        int d = m_data[n].time.date().day();
        int h = m_data[n].time.time().hour();
        if (d != dPrev) {dPrev = d; hPrev = 0;}
        if (h < hPrev) return false;
        hPrev = h;
    }
    return true;
}

} // namespace sp
