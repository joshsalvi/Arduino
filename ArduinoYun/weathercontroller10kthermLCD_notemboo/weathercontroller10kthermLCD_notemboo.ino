// Include required libraries
#include <Bridge.h>
#include <Process.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <math.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// Contains Temboo account information
#include "TembooAccount.h"

// Variables
int lightLevel;
float humidity;
float temperature;
float temperatureDHT;
unsigned long time;

// LED state in digital pin 13
// (D13) - - - (220ohm) - - - (LED) - - - (GND)

const int ledPin =  13;         // the number of the LED pin
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 1000;           // interval at which to blink (milliseconds)

#define ThermistorPIN 1                 // Analog Pin 0

float vcc = 4.91;                       // only used for display purposes, if used
// set to the measured Vcc.
float pad = 9850;                       // balance/pad resistor value, set this to
// the measured resistance of your pad resistor
float thermr = 10000;                   // thermistor nominal resistance

float Thermistor(int RawADC) {
  long Resistance;
  float Temp;  // Dual-Purpose variable to save space.

  Resistance = pad * ((1024.0 / RawADC) - 1);
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius

  double temp = (Temp * 9.0) / 5.0 + 32.0;          // Convert to Fahrenheit
  return Temp;                                      // Return the Temperature


}

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


// Process to get the measurement time
Process date;

// DHT11 sensor pins
#define DHTPIN 8
#define DHTTYPE DHT11

// DHT & BMP instances
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// Debug mode ?
boolean debug_mode = true;





void setup() {

  // Start Serial
  if (debug_mode == true) {
    Serial.begin(115200);
    delay(4000);
    while (!Serial);
  }

  // Initialize the LCD
  lcd.begin(16, 2);               // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.backlight();                // turn on backlight
  lcd.print("Temp./Humid.");      // print to the LCD

  // Initialize DHT sensor
  dht.begin();

  // Start bridge
  Bridge.begin();

  // Start date process
  time = millis();
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%D-%T");
    date.run();
  }

  if (debug_mode == true) {
    Serial.println("Setup complete. Waiting for sensor input...\n");
  }

  // Initialise the sensor
  if (!bmp.begin())
  {
    while (1);
  }

  pinMode(ledPin, OUTPUT);    // set LED pin number

}

void loop() {

  // Turn on LED and blink as status indicator
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }

  // Measure the humidity & temperature
  humidity = dht.readHumidity();
  temperatureDHT = dht.readTemperature();

  // Measure light level
  int lightLevel = analogRead(A0);

  // Measure temperature from sealed 10k thermistor
  // Provides temperature of the soil
  // Ground - - - (10k-resistor) - - - | - - - (Thermistor) - - - (+5V)
  //                                   |
  //                               Analog Pin 1
  float temp;
  temp = Thermistor(analogRead(ThermistorPIN));
  temp = (temp * 9.0) / 5.0 + 32.0;         // Convert from ºC to ºF

  // Measure pressure & temperature from BMP sensor
  sensors_event_t event;
  bmp.getEvent(&event);
  float pressure = event.pressure;

  bmp.getTemperature(&temperature);

  float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
  float altitude;
  altitude = bmp.pressureToAltitude(seaLevelPressure,
                                    event.pressure,
                                    temperature);

  if (debug_mode == true) {
    Serial.println("\nCalling the AppendRow Choreo...");
  }

  // Display humidity and temperature on LCD
  lcd.setCursor(0, 1);

  lcd.print("C=");
  lcd.print((float)temperature, 0);
  Serial.print("Temperature (oC): ");
  Serial.println((float)temperatureDHT, 2);

  lcd.print("  H=");
  lcd.print((float)humidity, 0);
  lcd.print("%");
  Serial.print("Humidity (%): ");
  Serial.println((float)humidity, 2);


  // Repeat every 100 ms
  delay(100);
}
