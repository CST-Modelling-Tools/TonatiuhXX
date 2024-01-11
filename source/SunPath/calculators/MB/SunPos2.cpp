// This file is available in electronic form at http://www.psa.es/sdg/sunpos.htm

#include "SunPath/calculators/MB/sunpos.h"
#include <math.h>
namespace sp {

void sunpos2(cTime udtTime,cLocation udtLocation, cSunCoordinates *udtSunCoordinates)
{
	// Main variables
	double dElapsedJulianDays;
	double dDecimalHours;
	double dEclipticLongitude;
	double dEclipticObliquity;
	double dRightAscension;
	double dDeclination;

	// Auxiliary variables
	double dY;
	double dX;

	// Calculate difference in days between the current Julian Day 
	// and JD 2451545.0, which is noon 1 January 2000 Universal Time
	{
		double dJulianDate;
		long int liAux1;
		long int liAux2;
		// Calculate time of the day in UT decimal hours
		dDecimalHours = udtTime.dHours + (udtTime.dMinutes 
			+ udtTime.dSeconds / 60.0 ) / 60.0;
		// Calculate current Julian Day
		liAux1 =(udtTime.iMonth-14)/12;
		liAux2=(1461*(udtTime.iYear + 4800 + liAux1))/4 + (367*(udtTime.iMonth 
			- 2-12*liAux1))/12- (3*((udtTime.iYear + 4900 
		+ liAux1)/100))/4+udtTime.iDay-32075;
		dJulianDate=(double)(liAux2)-0.5+dDecimalHours/24.0;
		// Calculate difference between current Julian Day and JD 2451545.0 
		dElapsedJulianDays = dJulianDate-2451545.0;
	}

	// Calculate ecliptic coordinates (ecliptic longitude and obliquity of the 
	// ecliptic in radians but without limiting the angle to be less than 2*Pi 
	// (i.e., the result may be greater than 2*Pi)
	{
		double dMeanLongitude;
		double dMeanAnomaly;
		double dOmega;
        dOmega = 2.267127827 - 9.300339267e-4*dElapsedJulianDays;
        dMeanLongitude = 4.895036035 + 1.720279602e-2*dElapsedJulianDays; // Radians
        dMeanAnomaly = 6.239468336 + 1.720200135e-2*dElapsedJulianDays;
        dEclipticLongitude = dMeanLongitude + 3.338320972e-2*sin( dMeanAnomaly )
            + 3.497596876e-4*sin(2*dMeanAnomaly) - 1.544353226e-4
            - 8.689729360e-6*sin(dOmega);
        dEclipticObliquity = 4.090904909e-1 - 6.213605399e-9*dElapsedJulianDays
            + 4.418094944e-5*cos(dOmega);
	}

	// Calculate celestial coordinates ( right ascension and declination ) in radians 
	// but without limiting the angle to be less than 2*Pi (i.e., the result may be 
	// greater than 2*Pi)
	{
		double dSin_EclipticLongitude;
		dSin_EclipticLongitude= sin( dEclipticLongitude );
		dY = cos( dEclipticObliquity ) * dSin_EclipticLongitude;
		dX = cos( dEclipticLongitude );
		dRightAscension = atan2( dY,dX );
		if( dRightAscension < 0.0 ) dRightAscension = dRightAscension + twopi;
		dDeclination = asin( sin( dEclipticObliquity )*dSin_EclipticLongitude );
	}

	// Calculate local coordinates ( azimuth and zenith angle ) in degrees
	{
		double dGreenwichMeanSiderealTime;
		double dLocalMeanSiderealTime;
		double dLatitudeInRadians;
		double dHourAngle;
		double dCos_Latitude;
		double dSin_Latitude;
		double dCos_HourAngle;
		double dParallax;
        dGreenwichMeanSiderealTime = 6.697096103 +
            6.570984737e-2*dElapsedJulianDays
			+ dDecimalHours;
		dLocalMeanSiderealTime = (dGreenwichMeanSiderealTime*15 
			+ udtLocation.dLongitude)*rad;
		dHourAngle = dLocalMeanSiderealTime - dRightAscension;
		dLatitudeInRadians = udtLocation.dLatitude*rad;
		dCos_Latitude = cos( dLatitudeInRadians );
		dSin_Latitude = sin( dLatitudeInRadians );
		dCos_HourAngle= cos( dHourAngle );
		udtSunCoordinates->dZenithAngle = (acos( dCos_Latitude*dCos_HourAngle
			*cos(dDeclination) + sin( dDeclination )*dSin_Latitude));
		dY = -sin( dHourAngle );
		dX = tan( dDeclination )*dCos_Latitude - dSin_Latitude*dCos_HourAngle;
		udtSunCoordinates->dAzimuth = atan2( dY, dX );
		if ( udtSunCoordinates->dAzimuth < 0.0 ) 
			udtSunCoordinates->dAzimuth = udtSunCoordinates->dAzimuth + twopi;
		udtSunCoordinates->dAzimuth = udtSunCoordinates->dAzimuth/rad;
		// Parallax Correction
		dParallax=(dEarthMeanRadius/dAstronomicalUnit)
			*sin(udtSunCoordinates->dZenithAngle);
		udtSunCoordinates->dZenithAngle=(udtSunCoordinates->dZenithAngle 
			+ dParallax)/rad;
	}
}

} // namespace sp
