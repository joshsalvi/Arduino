// Include required libraries
#include <Bridge.h>
#include <Process.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include "DHT.h"
#include <Process.h>
#include <Console.h>

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



// Process to get the measurement time
Process date;

// DHT11 sensor pins
#define DHTPIN 8
#define DHTTYPE DHT22

// DHT & BMP instances
DHT dht(DHTPIN, DHTTYPE);

// Debug mode ?
boolean debug_mode = false;





void setup() {
  Bridge.begin();
  Console.begin();
  // Start Serial
  if (debug_mode == true) {
    Serial.begin(115200);
    delay(4000);
    while (!Serial);
  }


  // Initialize DHT sensor
  dht.begin();


  // Start date process
  time = millis();
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%D-%T");
    date.run();
  }

  if (debug_mode == true) {
    Console.println("Setup complete. Waiting for sensor input...\n");
  }


  Console.println("Setup complete.");
  pinMode(ledPin, OUTPUT);    // set LED pin number

}

void loop() {
  digitalWrite(ledPin, HIGH);
  // Turn on LED and blink as status indicator
  unsigned long currentMillis = millis();

  // Measure the humidity & temperature
  humidity = dht.readHumidity();
  temperatureDHT = dht.readTemperature();
  Console.println("\n Humidity: ");
  Console.print(humidity);
  Console.println("\n Temperature: ");
  Console.print(temperatureDHT);

  if (debug_mode == true) {
    Console.println("\nCalling the AppendRow Choreo...");
  }
  digitalWrite(ledPin, LOW);


  // Repeat every 100 ms
  delay(5000);
}
