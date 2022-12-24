#include <SoftwareSerial.h> // Library for software serial
#include <OneWire.h>        // Library for 1-Wire devices
#include <Arduino.h>

// Define pin connections for sensors and microcontroller
#define TurbidityPin A2 // Pin for turbidity sensor
#define pHPin A0        // Pin for pH sensor
int pinRX = 2;          // RX pin for software serial
int pinTX = 3;          // TX pin for software serial

// Define constants for sensor calibration and interval times
#define Offset 0.00         // Deviation compensate value
#define samplingInterval 20 // Interval time for pH sensor sampling in milliseconds
#define printInterval 800   // Interval time for printing pH data in milliseconds
#define ArrayLength 40      // Number of times to collect pH data for averaging

// Create object for software serial communication
SoftwareSerial portSerial(pinRX, pinTX);

// Declare variables for sensor data
float voltageTBD, turbidity, voltagepH, pHValue, data;

// Declare array to store pH sensor data for averaging
int pHArray[ArrayLength];

// Declare variable to store index of current pH data in array
int pHArrayIndex = 0;

// Declare functions
void sendData(String name, float data);
void measurepH();
void measureTurbidity();
void printData(String name, float voltageData, float data);
double averageArray(int *arr, int number);

void setup()
{
    // Set up software serial ports
    portSerial.begin(9600);

    // Set up serial monitor
    Serial.begin(9600);
}

void loop()
{
    // Measure and send pH data
    measurepH();

    // Measure and send turbidity data
    measureTurbidity();
}

void measureTurbidity()
{
    // Read raw data from turbidity sensor
    int rawDataTBD = analogRead(A2);

    // Process the raw data to calculate the turbidity level
    voltageTBD = rawDataTBD * (5.0 / 1024);
    turbidity = 100.00 - (voltageTBD / 4.33) * 100.00;

    String TBDname = "Turbidity";

    // Print the processed data to the monitor
    printData(TBDname, voltageTBD, turbidity);

    // Send the processed data to the NodeMCU
    sendData(TBDname, turbidity);
}

void measurepH()
{
    // Declare variables to store the current time and the time when data was last read and printed
    static unsigned long samplingTime = millis();
    static unsigned long printTime = millis();

    // Check if it is time to read new data from the sensor
    if (millis() - samplingTime > samplingInterval)
    {
        // Read and process pH sensor data
        pHArray[pHArrayIndex++] = analogRead(pHPin);
        if (pHArrayIndex == ArrayLenth)
            pHArrayIndex = 0;

        voltagepH = averageArray(pHArray, ArrayLenth) * 5 / 1024;
        pHValue = 3.5 * voltagepH + Offset;

        // Update the sampling time
        samplingTime = millis();
    }

    // Check if it is time to print the processed data
    if (millis() - printTime > printInterval)
    {
        String pHname = "pH";

        // Print the processed data to the monitor
        printData(pHname, voltagepH, pHValue);

        // Send the processed data to the NodeMCU
        sendData(pHname, pHValue);

        // Update the print time
        printTime = millis();
    }
}

void printData(String name, float voltageData, float data)
{
    // Print the name, voltage, and data of the sensor to the monitor
    Serial.print(name);
    Serial.print(" = ");
    Serial.print("Voltage: ");
    Serial.print(voltageData, 2);
    Serial.print(" , ");
    Serial.print("Value: ");
    Serial.println(data, 3);

    // Delay to allow time for the data to be printed
    delay(500);
}

void sendData(String name, float data)
{
    // Check if the software serial port is available
    if (portSerial.available() > 0)
    {
        // Send the data to the NodeMCU
        portSerial.write(name);
        portSerial.write(data);
    }
}

double averageArray(int *arr, int number)
{
    // Declare variables
    int i;
    int max, min;
    double avg;
    long amount = 0;

    // Check if the number of elements in the array is valid
    if (number <= 0)
    {
        // Print error message and return 0 if number is invalid
        Serial.println("Error: Invalid number of elements in array!");
        return 0;
    }
    if (number < 5)
    {
        // Calculate average directly if number of elements is less than 5
        for (i = 0; i < number; i++)
        {
            amount += arr[i];
        }
        avg = amount / number;
        return avg;
    }
    else
    {
        // Find the minimum and maximum values in the array
        if (arr[0] < arr[1])
        {
            min = arr[0];
            max = arr[1];
        }
        else
        {
            min = arr[1];
            max = arr[0];
        }
        for (i = 2; i < number; i++)
        {
            if (arr[i] < min)
            {
                // Update min and add value to amount
                amount += min;
                min = arr[i];
            }
            else
            {
                if (arr[i] > max)
                {
                    // Update max and add value to amount
                    amount += max;
                    max = arr[i];
                }
                else
                {
                    // Add value to amount
                    amount += arr[i];
                }
            }
        }
        // Calculate average by summing all elements except for min and max and dividing by number of elements minus 2
        avg = (double)amount / (number - 2);
    }
    return avg;
}
