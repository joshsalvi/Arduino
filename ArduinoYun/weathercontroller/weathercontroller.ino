// Include required libraries
#include <Bridge.h>
#include <Temboo.h>
#include <Process.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// Contains Temboo account information
#include "TembooAccount.h"

// Variables
int lightLevel;
float humidity;
float temperature;
unsigned long time;

// Process to get the measurement time
Process date;

// Your Google Docs data
const String GOOGLE_CLIENT_ID = "577349964329-0qdh00tf0ksa8p28l8l2q93e29tnfedk.apps.googleusercontent.com";
const String GOOGLE_CLIENT_SECRET = "sbyGt_2QRt5TaKJNsB5uhWah";
const String GOOGLE_REFRESH_TOKEN = "1/756lHv3SZKaWdnQvtvHPpdKxuCvQ9u-eOdm3Y37UQ0U";
const String GOOGLE_USERNAME = "josh.salvi@gmail.com";
const String GOOGLE_PASSWORD = "Jdman716";
const String TO_EMAIL_ADDRESS = "josh.salvi@gmail.com";
const String SPREADSHEET_TITLE = "Weather Monitor";

// Include required libraries
#include "DHT.h"

// DHT11 sensor pins
#define DHTPIN 8
#define DHTTYPE DHT11

// DHT & BMP instances
DHT dht(DHTPIN, DHTTYPE);
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


}

void loop() {

  // Measure the humidity & temperature
  humidity = dht.readHumidity();

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

  if (debug_mode == true) {
    Serial.println("\nCalling the AppendRow Choreo...");
  }

  // Append data to Google Docs sheet
  runAppendRow(humidity, lightLevel, pressure, temperature, altitude);

  // Send email alert if temperature is too high
  int temperature_limit_high = 35;
  if (temperature > temperature_limit_high) {
    if (debug_mode == true) {
      Serial.println("Sending alert");
    }
    sendTempAlert("Temperature is too high!");
  }
  // Send email alert if temperature is too low
  int temperature_limit_low = 0;
  if (temperature < temperature_limit_low) {
    if (debug_mode == true) {
      Serial.println("Sending alert");
    }
    sendTempAlert("Temperature is too low!");
  }

  // Repeat every 3 hours
  delay(10800000);
}

// Function to add data to Google Docs
void runAppendRow(float humidity, int lightLevel,
                  float pressure, float temperature, float altitude) {
  TembooChoreo AppendRowChoreo;

  // Invoke the Temboo client
  AppendRowChoreo.begin();

  // Set Temboo account credentials
  AppendRowChoreo.setAccountName("joshsalvi");
  AppendRowChoreo.setAppKeyName("weatherlog");
  AppendRowChoreo.setAppKey("y4Ye9KbhbuXOW12nlmEJiGZaYbGdscPP");

  // Identify the Choreo to run
  AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");

  // your Google Client ID
  AppendRowChoreo.addInput("ClientID", "577349964329-0qdh00tf0ksa8p28l8l2q93e29tnfedk.apps.googleusercontent.com");

  // your Google Client Secret
  AppendRowChoreo.addInput("ClientSecret", "sbyGt_2QRt5TaKJNsB5uhWah");

  // your Google Refresh Token
  AppendRowChoreo.addInput("RefreshToken", "1/756lHv3SZKaWdnQvtvHPpdKxuCvQ9u-eOdm3Y37UQ0U");

  // the title of the spreadsheet you want to append to
  AppendRowChoreo.addInput("SpreadsheetTitle", "Weather Monitor");

  // Restart the date process:
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%D-%T");
    date.run();
  }

  // Get date
  String timeString = date.readString();

  // Format data
  String data = "";
  data = data + timeString + "," +
         String(humidity) + "," +
         String(lightLevel) + "," +
         String(pressure) + "," +
         String(temperature) + "," +
         String(altitude);

  // Set Choreo inputs
  AppendRowChoreo.addInput("RowData", data);

  // Run the Choreo
  unsigned int returnCode = AppendRowChoreo.run();

  // A return code of zero means everything worked
  if (returnCode == 0) {
    if (debug_mode == true) {
      Serial.println("Completed execution of the AppendRow Choreo.\n");
    }
  } else {
    // A non-zero return code means there was an error
    // Read and print the error message
    while (AppendRowChoreo.available()) {
      char c = AppendRowChoreo.read();
      if (debug_mode == true) {
        Serial.print(c);
      }
    }
    if (debug_mode == true) {
      Serial.println();
    }
  }
  AppendRowChoreo.close();
}

// Send email alert
void sendTempAlert(String message) {

  if (debug_mode == true) {
    Serial.println("Running SendAnEmail...");
  }

  TembooChoreo SendEmailChoreo;

  SendEmailChoreo.begin();

  // set Temboo account credentials
  SendEmailChoreo.setAccountName("joshsalvi");
  SendEmailChoreo.setAppKeyName("weatherlog");
  SendEmailChoreo.setAppKey("y4Ye9KbhbuXOW12nlmEJiGZaYbGdscPP");

  // identify the Temboo Library choreo to run (Google > Gmail > SendEmail)
  SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");

  // set the required choreo inputs
  // see https://www.temboo.com/library/Library/Google/Gmail/SendEmail/
  // for complete details about the inputs for this Choreo

  // the first input is your Gmail email address
  SendEmailChoreo.addInput("Username", "josh.salvi@gmail.com");
  // next is your Gmail password.
  SendEmailChoreo.addInput("Password", "Jdman716");
  // who to send the email to
  SendEmailChoreo.addInput("ToAddress", "josh.salvi@gmail.com");
  // then a subject line
  SendEmailChoreo.addInput("Subject", "ALERT: Temperature");

  // next comes the message body, the main content of the email
  SendEmailChoreo.addInput("MessageBody", "Temperature is too high or too low. See spreadsheet.");

  // tell the Choreo to run and wait for the results. The
  // return code (returnCode) will tell us whether the Temboo client
  // was able to send our request to the Temboo servers
  unsigned int returnCode = SendEmailChoreo.run();

  // a return code of zero (0) means everything worked
  if (returnCode == 0) {
    if (debug_mode == true) {
      Serial.println("Success! Email sent!");
    }
  } else {
    // a non-zero return code means there was an error
    // read and print the error message
    while (SendEmailChoreo.available()) {
      char c = SendEmailChoreo.read();
      if (debug_mode == true) {
        Serial.print(c);
      }
    }
  }
  SendEmailChoreo.close();
}

