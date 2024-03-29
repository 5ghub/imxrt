/*
  Calculating the Great Circle Distance and Course to a target position

  This example shows how to query a u-blox module for its latitude/longitude and then
  calculate the Great Circle Distance and Course to a target location.

  Thanks! distanceBetween and courseTo were taken from Mikal Hart's TinyGPSPlus:
  https://github.com/mikalhart/TinyGPSPlus/blob/ca29434514a5c5172bd807af0608df7f296582a2/src/TinyGPS%2B%2B.cpp#L285-L328

  Note: Lat/Long are large numbers because they are degrees * 10^-7. To convert lat/long
  to something google maps understands simply divide the numbers by 10,000,000. We 
  do this so that we don't have to use floating point numbers.

*/

#include <board.h>
#define DSerial SerialUSB

SFE_UBLOX_GNSS myGNSS;

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.

//#include <math.h> //Uncomment if required. May be needed for sqrt, atan2, etc..

double distanceBetween(long lat1_l, long long1_l, long lat2_l, long long2_l)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double lat1 = (double)lat1_l / 10000000.0; // Convert lat and long to degrees
  double long1 = (double)long1_l / 10000000.0;
  double lat2 = (double)lat2_l / 10000000.0;
  double long2 = (double)long2_l / 10000000.0;
  double delta = radians(long1-long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}

double courseTo(long lat1_l, long long1_l, long lat2_l, long long2_l)
{
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  // both specified as signed decimal-degrees latitude and longitude.
  // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
  // Courtesy of Maarten Lamers
  double lat1 = (double)lat1_l / 10000000.0; // Convert lat and long to degrees
  double long1 = (double)long1_l / 10000000.0;
  double lat2 = (double)lat2_l / 10000000.0;
  double long2 = (double)long2_l / 10000000.0;
  double dlon = radians(long2-long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double a1 = sin(dlon) * cos(lat2);
  double a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  return degrees(a2);
}

void setup()
{
  DSerial.begin(115200); // You may need to increase this for high navigation rates!
  while (DSerial.read() >= 0);
  delay(5000);
  while (!Serial); //Wait for user to open terminal
  DSerial.println("  u-blox Example");

  Wire.begin();

  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    DSerial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
}

void loop()
{
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
    
    long latitude = myGNSS.getLatitude();
    DSerial.print(F("Lat: "));
    DSerial.print(latitude);

    long longitude = myGNSS.getLongitude();
    DSerial.print(F(" Long: "));
    DSerial.print(longitude);
    DSerial.println(F(" (degrees * 10^-7)"));

    static const long TARGET_LAT =  478268712, TARGET_LON = -1222065397; 

    double distanceToTarget = distanceBetween(
    latitude,
    longitude,
    TARGET_LAT, 
    TARGET_LON);

    DSerial.print(F("Distance to target: "));
    DSerial.print(distanceToTarget, 2);
    DSerial.print(F(" (m)  "));
    
    double courseToTarget = courseTo(
    latitude,
    longitude,
    TARGET_LAT, 
    TARGET_LON);

    DSerial.print(F("Course to target: "));
    DSerial.print(courseToTarget, 1);
    DSerial.println(F(" (degrees)"));
  }
}
