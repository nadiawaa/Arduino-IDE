/*
    Lora Node 3
    myIpond V2

*/

// Include the necessary libraries
#include <Arduino.h>
#include <SPI.h>               // Libraries for LoRa
#include <LoRa.h>              // Libraries for LoRa
#include <Wire.h>              // Libraries for OLED Display
#include <Adafruit_GFX.h>      // Libraries for OLED Display
#include <Adafruit_SSD1306.h>  // Libraries for OLED Display
#include <OneWire.h>           // Library D18B20
#include <DallasTemperature.h> // Library D18B20

#include <FirebaseESP32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#define FIREBASE_HOST "change with firebasehost"              // Firebase hosts
#define WIFI_SSID "change with ssid wifi name"                // Wi-Fi name
#define WIFI_PASSWORD "change with password wifi"             // WiFi passwords
#define FIREBASE_Authorization_key "change with firebaseauth" // Firebase authentication code

// Define the pins used by the sensors
#define TempPin 13
#define pHPin 35
#define TbdPin 34

// Define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// Define the operating frequency band
#define BAND 915E6

// Define the OLED display pins and dimensions
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Define variables for pH sensor
unsigned long int avgValue; // Store the average value of the sensor feedback
float b;
int buf[10], temp;

// Define variables for turbidity sensor
float TbdVoltage;
int samples = 600;

// Define variables for temperature sensor
OneWire oneWire(TempPin);
DallasTemperature sensors(&oneWire);

// Declare a variable of type FirebaseData to store data from Firebase
FirebaseData firebaseData;

// Declare a variable of type FirebaseJson to store JSON data
FirebaseJson json;

// Create an OLED display object and initialize it
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Define variables to hold the sensor data
float TempData = 0;
float pHData = 0;
float TbdData = 0;

// Define a counter for the packets
int counter = 0;

// Declare function
float measureTemperature();
float measurePH();
float measureTurbidity();
float roundToDP(float in_value, int decimal_place);
void displayData(int counter, float sensorTemp, float sensorpH, float sensorTBD);
void uploadData();

const char *SERVER_NAME3 = "http://xxx/sensordata3.php";

// PROJECT_API_KEY is the exact duplicate of, PROJECT_API_KEY in config.php file
// Both values must be same
String PROJECT_API_KEY = "";

// Send an HTTP POST request every 30 seconds
unsigned long lastMillis = 0;
long interval = 5000;

void setup()
{
    // Initialize the serial monitor
    Serial.begin(9600);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting...");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Connect to Firebase
    Firebase.begin(FIREBASE_HOST, FIREBASE_Authorization_key);

    // Reset the OLED display
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    // Initialize the OLED display
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
    { // Address 0x3C for 128x32
        Serial.println("SSD1306 allocation failed");
        for (;;)
        {
        } // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LORA SENDER 3");
    display.display();
    Serial.println("LoRa Sender 3");

    // Initialize the SPI LoRa pins
    SPI.begin(SCK, MISO, MOSI, SS);
    // Setup the LoRa transceiver module
    LoRa.setPins(SS, RST, DIO0);

    if (!LoRa.begin(BAND))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
        {
        }
    }

    Serial.println("LoRa Initializing OK!");
    display.setCursor(0, 10);
    Serial.println("LoRa Initializing OK!");
    display.setCursor(0, 10);
    display.print("LoRa Initializing OK!");
    display.display();
    delay(2000);
}

void loop()
{
    // Check the WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
        // Check if it's time to send an HTTP POST request
        if (millis() - lastMillis > interval)
        {
            // Send the HTTP POST request
            uploadData();
            // Update the last time the request was sent
            lastMillis = millis();
        }
    }
    else
    {
        // Display a message if the WiFi is disconnected
        Serial.println("WiFi Disconnected");
    }

    // Restart the ESP if the counter reaches 300
    if (counter == 300)
    {
        ESP.restart();
    }
}

void uploadData()
{
    // Display the packet number being sent on the serial monitor
    Serial.print("Sending packet: ");
    Serial.println(counter);

    // Measure the sensor values
    float sensorTemp = measureTemperature();
    float sensorpH = measurePH();
    float sensorTurbidity = measureTurbidity();
    // Increment the counter
    counter++;

    // Display the data on the OLED display
    displayData(counter, sensorTemp, sensorpH, sensorTurbidity);

    // Convert float data types to strings
    String temp = String(sensorTemp);
    String pH = String(sensorpH);
    String turbidity = String(sensorTurbidity);
    // Convert int data type to string
    String counterStr = String(counter);

    // Prepare the data to be uploaded to Firebase
    String keys[4] = {"TEMPERATURE", "PH", "TBD", "counter"};
    String values[4] = {temp, pH, turbidity, counterStr};

    // Loop through the process of uploading the data to Firebase for each data
    for (int i = 0; i < 4; i++)
    {
        Firebase.setString(firebaseData, "sensor3/" + keys[i], values[i]);
    }

    // Prepare the data to be sent to the server
    String dataUpload;
    String keys_api[5] = {"api_key", "suhu3", "ph3", "kekeruhan3", "counter3"};
    String values_api[5] = {PROJECT_API_KEY, temp, pH, turbidity, counterStr};

    for (int i = 0; i < 5; i++)
    {
        dataUpload += keys_api[i] + "=" + values_api[i] + "&";
    }

    // Delete last "&" in string
    dataUpload = dataUpload.substring(0, dataUpload.length() - 1);

    // Start the connection to the server
    WiFiClient client;
    HTTPClient http;
    http.begin(client, SERVER_NAME3);

    // Add the content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Send the HTTP POST request
    int httpResponseCode3 = http.POST(dataUpload);

    // Display the server response code on the serial monitor
    Serial.print("HTTP Response code (3): ");
    Serial.println(httpResponseCode3);

    // Close the connection to the server
    http.end();

    // Delay for 10 seconds before looping again
    delay(10000);
}

// This function measures the temperature using a sensor and returns the temperature in Celsius as a float.
float measureTemperature()
{
    // Request temperature readings from the sensor
    sensors.requestTemperatures();

    // Get the temperature in Celsius from the sensor
    float tempData = sensors.getTempCByIndex(0);

    // Print the temperature to the serial monitor
    Serial.print("Temperature : ");
    Serial.println(tempData);

    // Return the temperature
    return tempData;
}

// This function measures the pH value using a sensor and returns the pH value as a float.
float measurePH()
{
    // Initialize variables for storing sensor readings
    int buf[10];
    int temp;
    int avgValue;

    // Read 10 values from the pH sensor
    for (int i = 0; i < 10; i++)
    {
        // Read the pH sensor value and store it in the buffer
        buf[i] = analogRead(pHPin);

        // Delay to allow the sensor to stabilize
        delay(10);
    }

    // Sort the pH sensor readings in ascending order
    for (int i = 0; i < 9; i++)
    {
        for (int j = i + 1; j < 10; j++)
        {
            if (buf[i] > buf[j])
            {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }

    // Calculate the average of the 6 center values in the buffer
    avgValue = 0;
    for (int i = 2; i < 8; i++)
    {
        avgValue += buf[i];
    }

    // Convert the average value to millivolts and pH units
    pHData = (float)avgValue * 5.0 / 6024 / 6;
    pHData = 3.5 * pHData;

    // Print the pH value to the serial monitor
    Serial.print("pH : ");
    Serial.println(pHData, 2);

    // Return the pH value
    return pHData;
}

// This function measures the turbidity using a sensor and returns the turbidity value as a float.
float measureTurbidity()
{
    // Initialize variables for storing sensor readings and calculated values
    float tbdVoltage = 0;
    int tbdData;

    // Read a specified number of samples from the turbidity sensor
    for (int i = 0; i < samples; i++)
    {
        // Read the turbidity sensor value and convert it to voltage
        tbdVoltage += ((float)analogRead(TbdPin) / 4880) * 5;
    }

    // Calculate the average voltage of the samples
    tbdVoltage = tbdVoltage / samples;

    // Round the voltage to 2 decimal places
    tbdVoltage = roundToDP(tbdVoltage, 2);

    // Calculate the turbidity value based on the voltage
    if (tbdVoltage < 2.5)
    {
        tbdData = 3000;
    }
    else
    {
        tbdData = -1120.4 * sq(tbdVoltage) + 5742.3 * tbdVoltage - 4352.9;
    }

    // Print the turbidity value to the serial monitor
    Serial.print("Turbidity: ");
    Serial.println(tbdData, 3);

    // Return the turbidity value
    return tbdData;
}

// This function displays data from the sensors on the OLED display.
void displayData(int counter, float sensorTemp, float sensorpH, float sensorTBD)
{
    // Clear the display and print sensor data
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LORA SENDER 3");
    display.setCursor(0, 10);
    display.print("LoRa packet sent.");
    display.setCursor(0, 20);
    display.print("Counter: ");
    display.print(counter);
    display.setCursor(0, 30);
    display.print("Temperature: ");
    display.print(sensorTemp);
    display.setCursor(0, 40);
    display.print("pH: ");
    display.print(sensorpH);
    display.setCursor(0, 50);
    display.print("Turbidity: ");
    display.print(sensorTBD);
    display.display();
}

// This function rounds a float value to a specified number of decimal places.
float roundToDP(float value, int decimalPlaces)
{
    // Calculate the multiplier to use for rounding
    float multiplier = powf(10.0f, decimalPlaces);

    // Round the value and return it
    value = roundf(value * multiplier) / multiplier;
    return value;
}
