#include <SoftwareSerial.h>
#include <OneWire.h> //adc


#define TD A2    //turbidity pin A2
#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection



SoftwareSerial uno(2,3); //komunikasi serial dengan nodemcu



int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;

static float kekeruhan;
static float teg;

void setup() {
  
uno.begin(9600);
Serial.begin(9600);

}

void loop() {
  
//turbidity______________________________________________________________________________

  int val = analogRead(A2);
  teg = val*(5.0/1024);
  kekeruhan = 100.00-(teg/4.33)*100.00;
  Serial.print(teg);
    Serial.print("       ");
  Serial.print("nilai adc= ");
  Serial.print(val);
     Serial.print("   ");
  Serial.print("kekeruhan= ");
  Serial.print(kekeruhan);
  Serial.println("     NTU");
  delay(500);
  

//ph_______________________________________________________________________________________________________

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5/1024;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {

    Serial.print("Voltage:");
        Serial.print(voltage,2);
        Serial.print("    pH value: ");
    Serial.println(pHValue,3);
        printTime=millis();
  }

if(uno.available()>0)
{
  uno.write(pHValue); //mengirim data ke nodemcu
  uno.write(kekeruhan);
}
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
