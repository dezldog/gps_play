//dezldog
// Playing with my gps and LCD
// some code mine, some code borrowed from Adafruit.
// Buy from them; they support the community. Alibaba vendors, etc, don't
//

#include "SoftwareSerial.h"
#include "Adafruit_GPS.h"
#include "Adafruit_LiquidCrystal.h"

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences.
#define GPSECHO  false

// Metric or not? Synthetica! metric = true, imperial = false
boolean displayUnits = true;

// create LCD object
Adafruit_LiquidCrystal lcd(0);

//Set up softwareserial
int rxPin = 8;
int txPin = 7;
int GPSBaud = 9600;

// Variables for formatting
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int sats = 0;
  float velocity = 0;

// create gps sensor connection object
SoftwareSerial gpsSerial(rxPin, txPin);

// create gps object
Adafruit_GPS GPS(&gpsSerial);

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy
uint32_t timer = millis();

void setup()
{
  //start gps
  Serial.begin(57600);
  gpsSerial.begin (GPSBaud);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  // Print a message to the LCD
  lcd.begin(16, 4);
  lcd.print("LCD Setup Complete");
  delay(3000);
  lcd.clear();
}

void loop()
{
  // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
  if (! usingInterrupt)
  {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
  }

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
    displayLcd();
    writeToSerial();
  }
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;
  // writing direct to UDR0 is much much faster than Serial.print
  // but only one character can be written at a time.
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void displayLcd()
{

  lcd.setCursor(17, 3);
  if ((int)GPS.fix)
  {
    lcd.print("Fix");
  }
  else
  {
    lcd.print("NFX");
  }
  velocity = GPS.speed;
  lcd.setCursor(0, 1);
  lcd.print("V=");
  if (velocity < 10 )
  {
    lcd.print("0");
  }
  lcd.print(velocity);
  lcd.setCursor(10, 1);
  lcd.print("Alt:"); lcd.print(GPS.altitude);
  lcd.setCursor(14, 2);
  sats = GPS.satellites;
  lcd.print("Sat:");
  if (sats < 10)
  {
    lcd.print("0");
    lcd.print(sats);
  }
  else
  {
    lcd.print(sats);
  }
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
  //Where to we print the date?
  lcd.setCursor(0, 0);
  lcd.print(GPS.month);
  lcd.print("/");
  lcd.print(GPS.day);
  lcd.print("/");
  lcd.print(GPS.year);
  lcd.print(" ");
  lcd.setCursor(0, 2);
  lcd.print("Lat:"); lcd.print(GPS.latitudeDegrees, 6);
  lcd.setCursor(0, 3);
  lcd.print("Lon:"); lcd.print(GPS.longitudeDegrees, 6);
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
    Serial.print("Angle: "); Serial.println(GPS.angle);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);
    Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
  }
}

