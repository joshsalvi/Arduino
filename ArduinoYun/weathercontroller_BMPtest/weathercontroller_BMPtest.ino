// Include required libraries
#include <Bridge.h>
#include <Process.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <math.h>

// Contains Temboo account information
#include "TembooAccount.h"

// Variables
int lightLevel;
float humidity;
float temperature;
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

// Process to get the measurement time
Process date;


// BMP instance
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// Debug mode ?
boolean debug_mode = false;


void setup() {

  // Start Serial
  if (debug_mode == true) {
    Serial.begin(115200);
    delay(4000);
    while (!Serial);
  }


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

  // Measure light level
  int lightLevel = analogRead(A0);


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

  Serial.println(temperature);
  Serial.println(pressure);

  if (debug_mode == true) {
    Serial.println("\nCalling the AppendRow Choreo...");
  }

  // Repeat every 100 ms
  delay(100);
}
