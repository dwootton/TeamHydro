//Libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include "Adafruit_SI1145.h"

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
//Not used in aquaponics, but could be useful as a template.
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
void printData()
{
  Serial.print("Air Temperature F ");
  Serial.println(calculateTemp("TemperatureF"));
  Serial.print("Humidity ");
  Serial.println(calculateTemp("Humidity"));
  Serial.print("UV Light ");
  Serial.println(calculateSunlight("UV"));
  Serial.print("Visible Light ");
  Serial.println(calculateSunlight("Visible"));
  Serial.print("IR Light ");
  Serial.println(calculateSunlight("IR"));
  Serial.print("UV Light ");
  Serial.println(calculateSunlight("UV"));
}

//Setup Function
void setup()
{
  // Debug console
  Serial.begin(9600);
  Serial.println("Hello! I am WaterWidget, at your service.");
  Serial.println("Now Reading Moisture Sensor Levels.");
  
  dht.begin(); 
  timer.setInterval(refreshRate, upload);
  
   if (! uv.begin()) {
    Serial.println("Didn't find Si1145");
    while (1);
  }
}

//Loop Function, Only Calls Timer
void loop()
{
printData();
delay(5000);
}
