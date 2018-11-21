//Libraries
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include "Adafruit_SI1145.h"

//Wifi Information and Auth Token from App
char auth[] = "2421e5a24df245ce9e2dc8954e6ad5a0";
char ssid[] = "FPR";
char pass[] = "cutthroat";

//Instantiate Timer
SimpleTimer timer;

//Define Virtual Pins to be Used With App
#define TableVPIN V0
#define MoistureVPIN V1
#define MoisturePercentVPIN V2
#define MoistureRawVPIN V3
#define TemperatureVPIN V4
#define HumidityVPIN V5
#define UVVPIN V6

//Define Sensor Pins
#define DHTPIN            D6
#define DHTTYPE           DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_SI1145 uv = Adafruit_SI1145();

//Reference Data and Interval
float referenceV = 3.200;
int refreshRate = 5000L;

//Calculates and Returns Temperature and Humidity Data Based on String Selection
float calculateTemp(String x)
{
sensors_event_t event;  
  if(x == "TemperatureC"){
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" *C");
    return (event.temperature);
  }
  }
  else if(x == "TemperatureF"){
    float temperatureF = (((9*calculateTemp("TemperatureC"))/5)+32);
    Serial.println(temperatureF);
    return temperatureF;
  }
  else if(x == "Humidity"){
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
    }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
    return (event.relative_humidity);
  }
  }
}

//Calculates and Returns Moisture Data Based on String Selection
float calculateMoisture(String x)
{
  if(x == "Voltage"){
float analogIn = analogRead(A0);
float scaledAnalogIn = analogIn/1023;
float analogVoltage = referenceV * scaledAnalogIn;
return analogVoltage;
  }
  else if(x == "Raw"){
    float analogIn = analogRead(A0);
    return int (analogIn); 
  }
  else if(x == "Percent"){
  float analogIn = analogRead(A0);
  float scaledAnalogInverted = (1023 - analogIn)/1023;
  int percentage = int((scaledAnalogInverted*100));  
  return percentage;
  }
  else if(x == "InvertedVoltage"){
    float analogIn = analogRead(A0);
    float scaledAnalogIn = analogIn/1023;
    float analogVoltage = referenceV * scaledAnalogIn;
    float analogVoltageInverted = referenceV - analogVoltage;
    return analogVoltageInverted;
  }
}

//Calculates and Returns Sunlight Data Based on String Selection
float calculateSunlight(String x)
{
  if(x == "Visible"){
    return uv.readVisible();
  }
  else if(x == "IR"){
    return uv.readIR();
  }
  else if(x == "UV"){
    float UVRaw = uv.readUV();
    float UVIndex = UVRaw/100;
    return UVIndex;
  }
}

//Function Triggered On Timmer, Uploads Data
void upload()
{
  Blynk.virtualWrite(MoistureVPIN, calculateMoisture("Voltage"));
  Blynk.virtualWrite(MoisturePercentVPIN, calculateMoisture("Percent"));
  Blynk.virtualWrite(MoistureRawVPIN, calculateMoisture("Raw"));
  Blynk.virtualWrite(TemperatureVPIN, calculateTemp("TemperatureF"));
  Blynk.virtualWrite(HumidityVPIN, calculateTemp("Humidity"));
  Blynk.virtualWrite(UVVPIN, calculateSunlight("UV"));
  Blynk.virtualWrite(TableVPIN, "update", 0, "Moisture Voltage", calculateMoisture("Voltage"));
  Blynk.virtualWrite(TableVPIN, "update", 1, "Moisture Inverted Voltage", calculateMoisture("InvertedVoltage"));
  Blynk.virtualWrite(TableVPIN, "update", 2, "Moisture Raw", calculateMoisture("Raw"));
  Blynk.virtualWrite(TableVPIN, "update", 3, "Moisture Percent", calculateMoisture("Percent"));
  Blynk.virtualWrite(TableVPIN, "update", 4, "Relative Humidity", calculateTemp("Humidity"));
  Blynk.virtualWrite(TableVPIN, "update", 5, "Temperature C", calculateTemp("TemperatureC"));
  Blynk.virtualWrite(TableVPIN, "update", 6, "Temperature F", calculateTemp("TemperatureF"));
  Blynk.virtualWrite(TableVPIN, "update", 7, "Visible Light", calculateSunlight("Visible"));
  Blynk.virtualWrite(TableVPIN, "update", 8, "IR Light", calculateSunlight("IR"));
  Blynk.virtualWrite(TableVPIN, "update", 9, "UV Index", calculateSunlight("UV"));
  Serial.println(calculateMoisture("Raw"));
}

//Setup Function
void setup()
{
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
  // Debug console
  Serial.begin(9600);
  Serial.println("Hello! I am WaterWidget, at your service.");
  Serial.println("Now Reading Moisture Sensor Levels.");
  dht.begin(); 
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(refreshRate, upload);
  Blynk.virtualWrite(V0, "add", 0, "Moisture Voltage", "init");
  Blynk.virtualWrite(V0, "add", 1, "Moisture Inverted Voltage", "init");
  Blynk.virtualWrite(V0, "add", 2, "Moisture Raw", "init");
  Blynk.virtualWrite(V0, "add", 3, "Moisture Percentage", "init");
  Blynk.virtualWrite(V0, "add", 4, "Relative Humidity", "init");
  Blynk.virtualWrite(V0, "add", 5, "Temperature C", "init");
  Blynk.virtualWrite(V0, "add", 6, "Temperature F", "init");
  Blynk.virtualWrite(V0, "add", 7, "Visible Light", "init");
  Blynk.virtualWrite(V0, "add", 8, "IR Light", "init");
  Blynk.virtualWrite(V0, "add", 9, "UV Index", "init");
   if (! uv.begin()) {
    Serial.println("Didn't find Si1145");
    while (1);
  }
}

//Loop Function, Only Calls Timer
void loop()
{
  Blynk.run();
  timer.run();
}

