#include <ESP8266WiFi.h> 
#include <FirebaseArduino.h>

#include <OneWire.h> //adc
#include <DallasTemperature.h> //DS18B20 sensor suhu
#include <NewPing.h> //SR04T sensor ultrasonic

#include <SoftwareSerial.h>
SoftwareSerial mcu(D1,D2);



#define FIREBASE_HOST "my-i-pond-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "VqaLWbyEY9nEly5jyO3NZjAotLkAXHMfmRzXN0b5"
#define WIFI_SSID "Abhyasa"
#define WIFI_PASSWORD "DRA012108"
/*
 # This sample code is used to test the pH meter V1.0.
 # Editor : YouYou
 # Ver    : 1.0
 # Product: analog pH meter
 # SKU    : SEN0161
*/

#define ONE_WIRE_BUS D4//2 //Suhu D2
#define trigPin D5 //Ultrasonic Trig D3
#define echoPin D6 //Ultrasonic Echo D4
#define MAX_DISTANCE 400 //Jarak maks Ultrasonic
NewPing sonar = NewPing(trigPin, echoPin, MAX_DISTANCE);

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire device    
DallasTemperature sensors(&oneWire); // Pass oneWire reference to DallasTemperature library

int pHValue;
int kekeruhan;
void setup(void)
{
  Serial.begin(9600);
  mcu.begin(9600);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
 
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

}

void loop(void)
{



//Temperature
  sensors.requestTemperatures();   
    Serial.print("Temperature  : ");  
    Serial.print(sensors.getTempCByIndex(0)); 
    Serial.println(" C"); 
    Firebase.setFloat("titik2/suhu", sensors.getTempCByIndex(0));
  delay(50); 

//Ultrasonic
  int jarak = sonar.ping_cm();
    Serial.print("Jarak        : ");
    Serial.print(jarak);
    Serial.println("cm");
    Firebase.setFloat("titik2/jarak", jarak);
  delay(50);

   
mcu.write("mcu");
if (mcu.available()>0)
{
  

//ph
  pHValue=mcu.read();
    Serial.print("pH value: ");
    Serial.println(pHValue);
    Firebase.setFloat("titik2/ph", pHValue);

//turbidity
  kekeruhan=mcu.read();
    Serial.print("turbidity: ");
    Serial.println(kekeruhan);
    Firebase.setFloat("titik2/turbidity", kekeruhan);
}
}
