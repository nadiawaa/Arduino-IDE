#include <SoftwareSerial.h>
#include <OneWire.h> 

// Define connections pin to sensor and mikrocontroler
#define TurbidityPin A2
#define pHPin A0
int pinRX = 2;
int pinTX = 3;

#define Offset 0.00  // deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth 40 // times of collection

// Object to represent software serial ports
SoftwareSerial portSerial(pinRX, pinTX);

// Declare a variable to store the average value of the sensor feedback
int pHArray[ArrayLenth];

//Declare a variable to store the counter of sensor
int pHArrayIndex = 0;

void setup()
{
  // Set up software serial ports
  portSerial.begin(9600);

  // Set up serial monitor
  Serial.begin(9600);
}

void TBDsensor(){
  // Read raw data from turbidity sensor
  int rawDataTBD = analogRead(A2);

  // Proccess raw data
  static float voltageTBD = rawDataTBD * (5.0 / 1024);
  static float turbidity = 100.00 - (voltageTBD / 4.33) * 100.00;

  // Print on monitor
  Serial.print(voltageTBD);
  Serial.print(" , ");
  Serial.print("raw data TBD : ");
  Serial.print(rawDataTBD);
  Serial.print(" , ");
  Serial.print("data TBD : ");
  Serial.print(turbidity);
  Serial.println(" NTU");
  delay(500);

}

void pHsensor(){
  // Define time to read data
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();

  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(pHPin);
    if (pHArrayIndex == ArrayLenth)
      pHArrayIndex = 0;

    static float voltagepH = avergearray(pHArray, ArrayLenth) * 5 / 1024;
    static float pHValue = 3.5 * voltagepH + Offset;

    samplingTime = millis();
  }
  if (millis() - printTime > printInterval) // Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {

    Serial.print("Voltage:");
    Serial.print(voltagepH, 2);
    Serial.print("    pH value: ");
    Serial.println(pHValue, 3);
    printTime = millis();
  }

  if (uno.available() > 0)
  {
    uno.write(pHValue); // mengirim data ke nodemcu
    uno.write(kekeruhan);
  }

}
void loop()
{

  
}

double avergearray(int *arr, int number)
{
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0)
  {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5)
  { // less than 5, calculated directly statistics
    for (i = 0; i < number; i++)
    {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else
  {
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
        amount += min; // arr<min
        min = arr[i];
      }
      else
      {
        if (arr[i] > max)
        {
          amount += max; // arr>max
          max = arr[i];
        }
        else
        {
          amount += arr[i]; // min<=arr<=max
        }
      } // if
    }   // for
    avg = (double)amount / (number - 2);
  } 
  return avg;
}
