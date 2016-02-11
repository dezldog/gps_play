// dezldog
// Playing with my gps and LCD
// some code mine, some code borrowed from Adafruit.
//

#include "Adafruit_GPS.h"
#include "Adafruit_LiquidCrystal.h"

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences.
#define GPSECHO  false

// Metric or not? Synthetica! metric = true, imperial = false
boolean displayUnits = true;

// Create LCD object
Adafruit_LiquidCrystal lcd(0);

#define gpsSerial Serial1

Adafruit_GPS GPS(&gpsSerial);

// Variables for formatting
int hours = 0;
int minutes = 0;
int seconds = 0;
int sats = 0;
float velocity = 0;

//Other variables
uint32_t timer = millis();
int GPSBaud = 9600;

// This keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
//void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

void setup()
{
  // Start gps
  Serial.begin(115200);
  GPS.begin(GPSBaud);
  gpsSerial.begin (GPSBaud);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  usingInterrupt = false;

  // Setup LCD and print a fun message
  lcd.begin(16, 4);
  lcd.print("Welcome to Dezldog");
  lcd.setCursor(0, 1);
  lcd.print("Start-Up Complete");
  lcd.setCursor(0, 2);
  lcd.print("some sort version #");
  lcd.setCursor(0, 3);
  for (int x = 0; x < 20; x++)
  {
    lcd.setCursor(x, 3);
    lcd.print("0");
    delay(200);
    lcd.setCursor(x - 1, 3);
    lcd.print(" ");
  }
  lcd.clear();
  
  // Setup labels so they don't have to written every update
  lcd.setCursor(0, 1);
  lcd.print("V=");
  lcd.setCursor(10, 1);
  lcd.print("Alt:");
  lcd.setCursor(0, 2);
  lcd.print("Lat:");
  lcd.setCursor(14, 2);
  lcd.print("Sat:");
  lcd.setCursor(0, 3);
  lcd.print("Lon:");

}

void loop()
{
 
    char c = GPS.read();
    
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
 
  if (GPS.newNMEAreceived())
    {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
    }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 1 seconds or so, print out the current stats
  if (millis() - timer > 1000)
    {
    timer = millis(); // reset the timer

    //write to the LCD
    displayLcd();

    // send serial
    writeToSerial();
    }
}

void displayLcd()
{
  //Where to we print the date?
  lcd.setCursor(0, 0);
  lcd.print(GPS.month);
  lcd.print("/");
  lcd.print(GPS.day);
  lcd.print("/");
  lcd.print(GPS.year);
    
  // Where are we ging to show the time?
  lcd.setCursor(9, 0);
  hours = GPS.hour;
  if (hours < 10)
    {
    lcd.print("0");
    lcd.print(hours);
    }
  else
    {
    lcd.print(hours);
    }
  lcd.print(':');
  minutes = GPS.minute;
  if (minutes < 10)
    {
    lcd.print("0");
    lcd.print(minutes);
    }
  else
    {
    lcd.print(GPS.minute);
    }
  lcd.print(':');
  seconds = GPS.seconds;
  if (seconds < 10)
    {
    lcd.print("0");
    lcd.print(seconds);
    }
  else
    {
    lcd.print(seconds);
    }
  lcd.print("UTC");

  //Show the velocity
  velocity = GPS.speed;
  lcd.setCursor(2, 1);
  if (velocity < 10 )
    {
    lcd.print("0");
    }
  lcd.print(velocity);

  //Show Altitude
  lcd.setCursor(15, 1);
  lcd.print(GPS.altitude);

  //Display the Latitude
  lcd.setCursor(4, 2);
  lcd.print(GPS.latitudeDegrees, 6);

  // How many satellites are we using?
  lcd.setCursor(18, 2);
  sats = GPS.satellites;
  if (sats < 10)
    {
    lcd.print("0");
    lcd.print(sats);
    }
  else
    {
    lcd.print(sats);
    }
  
  //Display Longitude
  lcd.setCursor(4, 3);
  lcd.print(GPS.longitudeDegrees, 6);

  //Do we have a fix?
  lcd.setCursor(17, 3);
  if ((int)GPS.fix)
    {
    lcd.print("Fix");
    }
  else
    {
    lcd.print("NFX");
    }
}

void writeToSerial()
  {
  Serial.print("\nTime: ");
  Serial.print(GPS.hour, DEC); Serial.print(':');
  Serial.print(GPS.minute, DEC); Serial.print(':');
  Serial.print(GPS.seconds, DEC); Serial.println();
  Serial.print("Date: ");
  Serial.print(GPS.day, DEC); Serial.print('/');
  Serial.print(GPS.month, DEC); Serial.print("/20");
  Serial.println(GPS.year, DEC);
  Serial.print("Fix: "); Serial.print((int)GPS.fix);
  Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
  if (GPS.fix)
    {
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", ");
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    Serial.print("Location (in degrees.decimals): ");
    Serial.print(GPS.latitudeDegrees, 6);
    Serial.print(", ");
    Serial.println(GPS.longitudeDegrees, 6);
    Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);
    Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }
