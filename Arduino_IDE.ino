#include <OneWire.h> //adc
#include <DallasTemperature.h> //DS18B20 sensor suhu
#include <NewPing.h> //SR04T sensor ultrasonic

#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
#define ONE_WIRE_BUS 2 //Suhu D2
#define trigPin 3 //Ultrasonic Trig D3
#define echoPin 4 //Ultrasonic Echo D4
#define MAX_DISTANCE 400 //Jarak maks Ultrasonic
NewPing sonar = NewPing(trigPin, echoPin, MAX_DISTANCE);
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire device    
DallasTemperature sensors(&oneWire); // Pass oneWire reference to DallasTemperature library
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;

void setup(){
  sensors.begin();
  Serial.begin(9600); 
}

void loop(){
  
//Turbidity
  int sensorValue = analogRead(A1);
  int turbidity = map(sensorValue, 650,940, 100, 0);
    Serial.print("Tubidity     : ");
    Serial.println(turbidity);
  delay(50);

//Temperature
  sensors.requestTemperatures();   
    Serial.print("Temperature  : ");  
    Serial.print(sensors.getTempCByIndex(0)); 
    Serial.println(" C"); 
  delay(50); 

//Ultrasonic
  int jarak = sonar.ping_cm();
    Serial.print("Jarak        : ");
    Serial.print(jarak);
    Serial.println("cm");
  delay(50);

//PH
static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    Serial.print("Voltage:");
        Serial.print(voltage,2);
        Serial.print("    pH value: ");
    Serial.println(pHValue,2);
        digitalWrite(LED,digitalRead(LED)^1);
        printTime=millis();
  }
   Serial.println("----------------------------");
 delay(1000);

}
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;


} 
