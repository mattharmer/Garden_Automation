// Try to move login to network to below and logout each time (once an hour).
// Prob save power.

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid     = "DODO-8D7F";
const char* password = "5YGRMXGMFR";
const char* serverName = "http://mattharmer.me/post-sensor.php"; //Your Domain name with URL path or IP address with path
String apiKeyValue = "tPmAT5Ab3j7F9"; // If you change the apiKeyValue value, the PHP file /esp-post-data.php also needs to have the same key

// VARIABLES
int valueCount = 120;        // How many values to collect. Should be every hour.
int valueTime = 29500;     // Time between value counts. currently 29.5 seconds.
// Every 1248 SQL rows is one year. I should make new table every year.
int moistureChange = 1015; // The value between dry and wet that the relay swithces at
const int dry = 1024;      // When Capacitive Soil Moisture Sensor is Dry
const int wet = 940;       // Will need to recalibrate in soil

#define DHT11PIN 13   // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHT11PIN, DHTTYPE); // Initialize DHT sensor.
float DHT11tval;
float DHT11hval;

int TZTpin = 5;       // Variable for Light Detector
int TZTval = 0;     // Set initial value (Maybe not needed)
int TZTshd = 0;     // Number of shade reads
int TZTfin = 0;     // Final minutes in shade (number of reads times 2)

int CSMSV2pin = 0;
int CSMSV2rel = 0;    // variable for relay on/off check
int CSMSV2pct = 0;    // Moisture as Percentage
int CSMSV2sum = 0;    // Sum of all the reads
int CSMSV2fin = 0;    // Final minutes relay on (Number of reads divided by total reads)

const int RELAY01pin = 14;
int RELAY01val = 0;
int RELAY01num = 0; // Total number of relay on's.
int RELAY01tim = 0; // Total Operation time of Relay, hence water running time.

const int DS18B20pin = 12;
float DS18B20val = 0;
#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature SoilTempSensor(&oneWire);
DeviceAddress insideThermometer, outsideThermometer;
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

int sensorCount = 0; // The current count of sensor reads

void setup() {

  Serial.begin(115200);

  pinMode(TZTpin,INPUT);
  pinMode(DS18B20pin, INPUT);
  pinMode(RELAY01pin, OUTPUT);

  // DS18B20 Setup
  Serial.println(F("DS18B20 Soil Temp Sensor"));
  SoilTempSensor.begin();
  Serial.print(SoilTempSensor.getDeviceCount(), DEC);
  Serial.println(" devices.");
  if (!SoilTempSensor.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  SoilTempSensor.setResolution(insideThermometer, 9);
  Serial.print("Device 0 Resolution: ");
  Serial.print(SoilTempSensor.getResolution(insideThermometer), DEC); 
  Serial.println(F("------------------------------------"));

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
}

void printTemperature(DeviceAddress deviceAddress)
{
  DS18B20val = SoilTempSensor.getTempC(deviceAddress);
  if(DS18B20val == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Soil Temp: ");
  Serial.print(DS18B20val);
  Serial.println("°C");

}

void loop() { 
  
  int CSMSV2array[119]; // 120 Elements in array.
  byte CSMSV2index = 0;

  // Send Serial prints over local network to PC...

  // MySQL has a hard limit of 4096 lines per table.
  // This will keep the solinoid from being on for too long.
  // Might need to look into timer if 4 hours is not accurate.

  while (sensorCount < valueCount) { // Loop everything 6 times to get an average of 6 values
    delay(valueTime);   // Wait ~29.5 seconds between measurements.

    //TZT
    TZTval = analogRead(TZTpin); 
    Serial.print(TZTval, DEC); // light intensity
    if (TZTval > 1000) {
      TZTshd++;
      Serial.println(" - Shade");
      } else { 
        Serial.println(" - Sunny"); }
    Serial.print("TZT Shade Count: ");// Calculate this to get minute of shade...
    Serial.println(TZTshd);
  
    // CSMSV2  
    Serial.print("CSMSV2 Value: ");
    Serial.println(analogRead(CSMSV2pin));
    CSMSV2array[CSMSV2index] = analogRead(CSMSV2pin);    //put a value in entry 0
    CSMSV2rel = analogRead(CSMSV2pin); // A variable to work the relay

    // RELAY01
    if (CSMSV2rel > moistureChange) {
      digitalWrite(RELAY01pin,LOW);
      RELAY01val = 1;
      Serial.println("Min Moisture - Pump Running");
      } else {
        digitalWrite(RELAY01pin,HIGH);
        RELAY01val = 0;
        Serial.println("Max Moisture - Pump Stopped");
      }
    if (RELAY01val == 1) { RELAY01num++; } // Add one relay time increase
    Serial.print("Relay Switch Count: ");// Calculate this to get minute of operation...
    Serial.println(RELAY01num);

    Serial.println(F("------------------"));
    CSMSV2index++;
    sensorCount++;
  }

    // DS18B20 Hourly Reading
    SoilTempSensor.requestTemperatures(); // Send the command to get temperatures
    printTemperature(insideThermometer); // Use a simple function to print out the data

    // DHT11 Hourly Reading
    float DHT11tval = dht.readHumidity();       // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float DHT11hval = dht.readTemperature();    // Reading temperature or humidity takes about 250 milliseconds!
    if (isnan(DHT11tval) || isnan(DHT11hval)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return; }
    Serial.print(F("Humidity: "));
    Serial.print(DHT11tval);
    Serial.print(F("%  Temperature: "));
    Serial.print(DHT11hval);
    Serial.println(F("°C "));

    // CSMSV2 Array to average results to percent for hour
    for ( int i = 0; i < 120; i++ ) {
      CSMSV2sum += CSMSV2array[i]; }
    Serial.print("Sum of total elements of an array: ");
    Serial.println(CSMSV2sum);
    CSMSV2fin = CSMSV2sum / 120;
    Serial.print("Average of array: ");
    Serial.println(CSMSV2fin);
    CSMSV2pct = map(CSMSV2fin, wet, dry, 100, 0);
    Serial.print(" - Soil Moisture: ");
    Serial.print(CSMSV2pct);
    Serial.println("%");
    CSMSV2sum = 0;
    CSMSV2fin = 0;

    //Relay minutes of operation per hour
    RELAY01tim = RELAY01num / 2; // Divide by 2 to get minutes from 30 seconds.
    Serial.print("Total relay minutes:");
    Serial.println(RELAY01tim);
    RELAY01num = 0;

    //TZT minutes of shade per hour
    TZTfin = TZTshd / 2; // Divide by 2 to get minutes from 30 seconds.
    Serial.print("Total minutes of shade:");
    Serial.println(TZTfin);
    TZTshd = 0;

    if(WiFi.status()== WL_CONNECTED){ //Check WiFi connection status
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
      String httpRequestData = "api_key=" + apiKeyValue // Prepare your HTTP POST request data
      + "&valueDHT11t=" + String(dht.readTemperature())
      + "&valueDHT11h=" + String(dht.readHumidity())
      + "&valueTZT=" + String(TZTfin)
      + "&valueCSMSV2=" + String(CSMSV2pct)
      + "&valueDS18B20=" + String(DS18B20val)
      + "&valueRELAY01=" + String(RELAY01tim)
      + "";
      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData); 
      int httpResponseCode = http.POST(httpRequestData); // Send HTTP POST request

      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.println(F("------------------"));
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        Serial.println(F("------------------"));
      }
      // Free resources
      http.end();

      RELAY01tim = 0;
      TZTfin = 0;

    }
    else {
      Serial.println("WiFi Disconnected");
    }
  sensorCount = 0;
}

