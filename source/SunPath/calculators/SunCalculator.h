#pragma once

#include "Location.h"

#include <QDateTime>

// add sun calc
// https://www.aa.quae.nl/en/reken/zonpositie.html

namespace sp {


struct SunPath Horizontal: public vec2d
{
    Horizontal(double azimuth = 0., double elevation = 0.):
        vec2d(azimuth, elevation) {}

    double azimuth() const {return x;} // from North to East
    double elevation() const {return y;}

    void makeAzimuthPositive() {if (x < 0.) x += 2.*pi;}
};


struct SunPath Equatorial: public vec2d
{
    Equatorial(double hour = 0., double declination = 0.):
        vec2d(hour, declination) {}

    double hour() const {return x;} // hour angle
    double declination() const {return y;}
};

// vector to sun: x (East), y (North), z (Zenith)


class SunPath SunCalculator
{
public:
    SunCalculator();
    virtual SunCalculator* copy() const;
    virtual ~SunCalculator() {}

    // location
    void setLocation(const Location& location);
    const Location& location() const {return m_location;}
    QDateTime getLocalTime(const QDate& date, const QTime& time) const
        {return m_location.getLocalTime(date, time);}

    // position
    vec3d findVector(const Horizontal& hc) const;
    vec3d findVector(const Equatorial& ec) const;
    vec3d findVector(const QDateTime& t) const
        {return findVector(findHorizontalV(t));}

    Horizontal findHorizontal(const vec3d& v) const;
    Horizontal findHorizontal(const Equatorial& ec) const
        {return findHorizontal(findVector(ec));}
    virtual Horizontal findHorizontalV(const QDateTime& t) const; // needs V

    Equatorial findEquatorial(const vec3d& v) const;
    Equatorial findEquatorial(const Horizontal& hc) const
        {return findEquatorial(findVector(hc));}
    Equatorial findEquatorial(const QDateTime& t) const
        {return findEquatorial(findVector(t));}

    virtual QDateTime findTime(const Horizontal& hc, const QDateTime& t0) const;

    // auxiliary
    static double obliquity() {return s_obliquity;}
    static void setObliquity(double eps) {s_obliquity = eps;}

    // info
    virtual QString info() const {return "";}

protected:
    static double s_obliquity; // obliquity of ecliptic

    Location m_location; // location on Earth
    double m_cosPhi; // cosine of latitude
    double m_sinPhi; // sine of latitude
};

} // namespace sp
