
//Changelog: Altered the way moisture was calculated, and now turns on and off the moisture sensors in order to prevent corrosion. 

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
#include <Adafruit_NeoPixel.h>

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
#define LedVPIN V10


//Define Sensor Pins
int m_Enable = 0;
#define DHTPIN            D6
#define DHTTYPE           DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_SI1145 uv = Adafruit_SI1145();

// pH and Water Temp config
// Define pH Info
#define pHPin 3          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

#include <Wire.h> // one of these is depreciated
#include <OneWire.h> // one of these is depreciated
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature waterTempSensor(&oneWire);


//Define Neopixel Pins
#define LEDPIN 15
Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, LEDPIN, NEO_GRB + NEO_KHZ800);

BLYNK_WRITE(LedVPIN){
    int r = param[0].asInt();
    int g = param[1].asInt();
    int b = param[2].asInt();
   strip.Color(r,g,b); 
   strip.show();
  }
  
//Reference Data and Interval
float referenceV = 3.200;
int refreshRate = 10000L;

//Global Values
float moisture_analog_voltage;
float moisture_raw;
float moisture_percentage;
float moisture_inverted_voltage;

//Runs and Updates LED Icon
void LED(String Setting){
  if(Setting == "Update"){
    int  brightness = 200;
    int b = 0;
    for(b=0;b<brightness;b++){
    strip.setPixelColor(0, strip.Color(0,0,b));
    strip.setPixelColor(1, strip.Color(0,0,b));
    strip.setPixelColor(2, strip.Color(0,0,b));
    strip.setPixelColor(3, strip.Color(0,0,b));
    strip.setPixelColor(4, strip.Color(0,0,b));
    strip.show();
    delay(2);
    }
    for(b=brightness;b!=0;b--){
    strip.setPixelColor(0, strip.Color(0,0,b));
    strip.setPixelColor(1, strip.Color(0,0,b));
    strip.setPixelColor(2, strip.Color(0,0,b));
    strip.setPixelColor(3, strip.Color(0,0,b));
    strip.setPixelColor(4, strip.Color(0,0,b));
    strip.show();
    delay(2);    
  }
}
else if(Setting == "Pulse"){  
    int brightness = 50;
    int g = 0; 
    for(g=0;g<brightness;g++){
    strip.setPixelColor(0, strip.Color(0,g,0));
    strip.show();
    delay(2);
    }
    for(g=brightness;g>-1;g--){
    strip.setPixelColor(0, strip.Color(0,g,0));
    strip.show();
    delay(2);    
  }
}
}

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
float calculateMoisture(float & analogVoltage, float & analogIn, float & percentage, float & analogVoltageInverted)
{
digitalWrite(D7, HIGH);
Serial.println("Turning on Moisture Sensor Bank");
delay(2000);
analogIn = analogRead(A0);
float scaledAnalogIn = analogIn/1023;
analogVoltage = referenceV * scaledAnalogIn;
float scaledAnalogInverted = (1023 - analogIn)/1023;
percentage = int((scaledAnalogInverted*100)); 
analogVoltageInverted = referenceV - analogVoltage;
digitalWrite(D7, LOW);
Serial.println("Turning Off Moisture Sensor Bank");
}

//Calculates water Temp
float calculateTemp(){
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  waterTempSensor.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(waterTempSensor.getTempCByIndex(0));  
}

//Calculates pH
float calculatepH(){
  pinMode(13,OUTPUT);  
  Serial.begin(9600);  
  Serial.println("Ready");    //Test the serial monitor

  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(pHPin);
    delay(10);
  }

  int n = sizeof(buf)/sizeof(buf[0]); 
  
  sort(buf, buf+n); 
    
  float avgValue=0;
  
  for(int i=2;i<8;i++)                    //take the average value of 6 center sample
  {
    avgValue+=buf[i];
  }
  
  float phValue = avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue = 3.5*phValue;                      //convert the millivolt into pH value
  Serial.print("    pH:");  
  Serial.print(phValue,2);
  Serial.println(" ");
}

//Calculates and Returns Sunlight Data Based on String Selection
float calculateSunlight(String x)
{
  if(x == "Visable"){
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
  calculateMoisture(moisture_analog_voltage, moisture_raw, moisture_percentage, moisture_inverted_voltage);
  Blynk.virtualWrite(MoistureVPIN, moisture_analog_voltage);
  Blynk.virtualWrite(MoisturePercentVPIN, moisture_percentage);
  Blynk.virtualWrite(MoistureRawVPIN, moisture_raw);
  Blynk.virtualWrite(TemperatureVPIN, calculateTemp("TemperatureF"));
  Blynk.virtualWrite(HumidityVPIN, calculateTemp("Humidity"));
  Blynk.virtualWrite(UVVPIN, calculateSunlight("UV"));
  Blynk.virtualWrite(TableVPIN, "update", 0, "Moisture Voltage", moisture_analog_voltage);
  Blynk.virtualWrite(TableVPIN, "update", 1, "Moisture Inverted Voltage", moisture_inverted_voltage);
  Blynk.virtualWrite(TableVPIN, "update", 2, "Moisture Raw", moisture_raw);
  Blynk.virtualWrite(TableVPIN, "update", 3, "Moisture Percent", moisture_percentage);
  Blynk.virtualWrite(TableVPIN, "update", 4, "Relative Humidity", calculateTemp("Humidity"));
  Blynk.virtualWrite(TableVPIN, "update", 5, "Temperature C", calculateTemp("TemperatureC"));
  Blynk.virtualWrite(TableVPIN, "update", 6, "Temperature F", calculateTemp("TemperatureF"));
  Blynk.virtualWrite(TableVPIN, "update", 7, "Visible Light", calculateSunlight("Visible"));
  Blynk.virtualWrite(TableVPIN, "update", 8, "IR Light", calculateSunlight("IR"));
  Blynk.virtualWrite(TableVPIN, "update", 9, "UV Index", calculateSunlight("UV"));
  LED("Pulse");
}

//Setup Function
void setup()
{
  // Addition for water temp and such
  waterTempSensor.begin();
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
pinMode(D7, OUTPUT);
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
  strip.begin();
  strip.show();
}

//Loop Function, Only Calls Timer
void loop()
{
  Blynk.run();
  timer.run();
}

