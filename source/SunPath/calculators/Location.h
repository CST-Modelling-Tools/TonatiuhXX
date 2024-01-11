#pragma once

#include "SunPath/math/geometry/vec3d.h"
#include <QString>
#include <QDateTime>

namespace sp {


class SunPath Location
{
public:
    Location(
        QString name = "",
        double latitude = 0., double longitude = 0.,
        int offsetUTC = 0
    ):
        m_name(name),
        m_location(longitude, latitude),
        m_offsetUTC(offsetUTC)
    {}

    void setName(QString name) {m_name = name;}
    QString name() const {return m_name;}

    void setLocation(double latitude, double longitude)
        {m_location = vec2d(longitude, latitude);}
    vec2d location() const {return m_location;}

    void setLatitude(double y) {m_location.y = y;}
    double latitude() const {return m_location.y;}

    void setLongitude(double x) {m_location.x = x;}
    double longitude() const {return m_location.x;}

    void setOffsetUTC(int offsetUTC) {m_offsetUTC = offsetUTC;}
    int offsetUTC() const {return m_offsetUTC;}

    QDateTime getLocalTime(const QDate& date, const QTime& time) const
        {return QDateTime(date, time, Qt::OffsetFromUTC, m_offsetUTC);}

protected:
    QString m_name;
    vec2d m_location;
    int m_offsetUTC; // offset from UTC in seconds
};


} // namespace sp
