#include <ESP8266WiFi.h>       // library for WiFi connection
#include <FirebaseArduino.h>   // library for connecting to Firebase
#include <OneWire.h>           // library for temperature sensors
#include <DallasTemperature.h> // library for temperature sensors
#include <SoftwareSerial.h>    // library for serial communication with the microcontroller

#define FIREBASE_HOST "firebasehost" // Firebase hosts
#define FIREBASE_AUTH "firebaseauth" // Firebase authentication code
#define WIFI_SSID "ssid"             // Wi-Fi name
#define WIFI_PASSWORD "pwd"          // WiFi passwords

// Define pin connections for sensors and microcontroller
#define ONE_WIRE_BUS D4 // Temperature sensor connection pins
#define pinRX D1        // RX pin for software serial
#define pinTX D2        // TX pin for software serial

// Create object for software serial communication
SoftwareSerial portSerial(pinRX, pinTX);

// Temperature sensor initialization
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Declare variables for sensor data
String name;
float value;

void sendData_to_firebase(String name, float data);
void measureTemp() void readData()

    void setup()
{
    // Start serial communication with baud rate of 9600
    Serial.begin(9600);

    // Initialize software serial with baud rate of 9600
    portSerial.begin(9600);

    // Connect to WiFi using the defined SSID and password
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Print message to serial monitor and wait until WiFi connection is established
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        // Print "." every 500ms until WiFi is connected
        Serial.print(".");
        delay(500);
    }

    // Print message to serial monitor once WiFi is connected
    Serial.println();
    Serial.print("Connected to WiFi: ");
    Serial.println(WiFi.localIP());

    // Connect to Firebase using the defined host and authentication code
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()
{
    measureTemp();
}

void measureTemp()
{
    // Request the temperature sensor to measure the current temperature
    sensors.requestTemperatures();

    // Print the string "Temperature : " to the serial port
    Serial.print("Temperature  : ");

    // Print the current temperature value to the serial port
    Serial.print(sensors.getTempCByIndex(0));

    // Set the value of the "name" variable to the string "Temperature"
    name = "Temperature";

    // Send the current temperature data to Firebase
    sendData_to_firebase(name, sensors.getTempCByIndex(0));

    // Call readData function
    readData();

    // Wait for 50 milliseconds
    delay(50);
}

void readData()
{
    // Send request to Arduino
    portSerial.write("mcu");

    // Check if there is data available from Arduino
    if (portSerial.available() > 0)
    {
        // Read data from NodeMCU
        name = portSerial.read();
        value = portSerial.read();

        // Print data name and value to the serial monitor
        Serial.print(name + " value: ");
        Serial.println(value);

        // Send the current temperature data to Firebase
        sendData_to_firebase(name, value);
    }
}

void sendData_to_firebase(String name, float data)
{

    // Set data value to the corresponding data name in Firebase
    Firebase.setFloat("node_one/" + name, value);
}
