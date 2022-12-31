/*
    Lora Node 1
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

// Create an OLED display object and initialize it
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Define a string to hold the outgoing message
String outgoing;

// Define the addresses for the Master Node and this node
byte MasterNode = 0xFF;
byte SenderNode = 0xBB; // Node1

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
void sendData(int counter, float sensorTemp, float sensorpH, float sensorTBD);
void displayData(int counter, float sensorTemp, float sensorpH, float sensorTBD);
void sendMessage(String outgoing, byte MasterNode, byte otherNode);

// This function is called once on startup to initialize the OLED display, serial monitor, and LoRa transceiver module.
void setup()
{
    // Initialize the serial monitor
    Serial.begin(9600);

    // Reset the OLED display through software
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    // Initialize the OLED display
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
    {
        // Address 0x3C for 128x32
        Serial.println("SSD1306 allocation failed");
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Clear the display and print a message
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LoRa Sender 1");
    display.display();
    Serial.println("LoRa Sender 1");

    // Initialize the SPI pins for the LoRa transceiver module
    SPI.begin(SCK, MISO, MOSI, SS);

    // Set the pins for the LoRa module and begin communication
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(BAND))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }

    // Print a message indicating that the LoRa module was successfully initialized
    Serial.println("LoRa Initializing OK!");
    display.setCursor(0, 10);
    display.print("LoRa Initializing OK!");
    display.display();
    delay(2000);
}

// This function is called repeatedly to perform tasks such as measuring sensor values, sending data over the LoRa network, and displaying data on the OLED display.
void loop()
{
    // Measure the sensor values
    float sensorTemp = measureTemperature();
    float sensorpH = measurePH();
    float sensorTurbidity = measureTurbidity();

    // Print a message and send the data over the LoRa network
    Serial.print("Sending packet: ");
    Serial.println(counter);
    sendData(counter, sensorTemp, sensorpH, sensorTurbidity);

    // Display the data on the OLED display
    displayData(counter, sensorTemp, sensorpH, sensorTurbidity);

    // Increment the counter and delay before the next iteration
    counter++;
    delay(300000);

    // If the counter reaches a certain value, reset the ESP
    if (counter == 300)
    {
        ESP.restart();
    }
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

// This function sends data from the specified sensors to the Master Node over the LoRa network.
void sendData(int counter, float sensorTemp, float sensorpH, float sensorTBD)
{
    // Create a message string to send over the LoRa network
    String SenderNodeMessage;
    SenderNodeMessage = SenderNodeMessage + sensorTemp + "," + sensorpH + "," + sensorTBD + "," + counter;

    // Begin a packet and send the message to the Master Node
    LoRa.beginPacket();
    LoRa.print(counter);
    sendMessage(SenderNodeMessage, MasterNode, SenderNode);
    LoRa.endPacket();
}

// This function displays data from the sensors on the OLED display.
void displayData(int counter, float sensorTemp, float sensorpH, float sensorTBD)
{
    // Clear the display and print sensor data
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LORA SENDER 1");
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

// This function sends a message over the LoRa network to the specified destination node.
void sendMessage(String outgoing, byte MasterNode, byte otherNode)
{
    // Begin a packet and add the destination and sender addresses, message ID, and payload length
    LoRa.beginPacket();            // start packet
    LoRa.write(MasterNode);        // add destination address
    LoRa.write(SenderNode);        // add sender address
    LoRa.write(counter);           // add message ID
    LoRa.write(outgoing.length()); // add payload length
    LoRa.print(outgoing);          // add payload
    LoRa.endPacket();              // finish packet and send it
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
