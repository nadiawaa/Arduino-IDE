/*
  Lora Node 2 
  myIpond
*/
//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Library D18B20
#include <OneWire.h>
#include <DallasTemperature.h>

//define the pins used by sensors
#define SUHUPIN 13
#define PHPIN 35
#define TBDPIN 34 

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BAND 915E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128        // OLED display width, in pixels
#define SCREEN_HEIGHT 64        // OLED display height, in pixels

//ph setting
unsigned long int avgValue;    //Store the average value of the sensor feedback
float b;
int buf[10],temp;

//tbd setting
float Turbidity_Sensor_Voltage;
int samples = 600;
float ntu;

//temp setting
OneWire oneWire(SUHUPIN); 
DallasTemperature sensors(&oneWire);


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String outgoing;              // outgoing message

byte MasterNode = 0xFF;     
byte Node2 = 0xCC;

float sensorsuhu = 0;
float sensorph = 0;
float sensortbd = 0;

//packet counter
int counter = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(9600);
  
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER 2");
  display.display();
  
  Serial.println("LoRa Sender 2");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void loop() {
   
  Serial.print("Sending packet: ");
  Serial.println(counter);

  //sensor temp
  sensors.requestTemperatures(); 
  Serial.print("Temperature is: "); 
  Serial.println(sensors.getTempCByIndex(0));
  delay(500);

  //sensor ph
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(PHPIN);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/6024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  Serial.print("pH:");  
  Serial.print(phValue,2);
  Serial.println(" ");

  //sensor turbidity
  Turbidity_Sensor_Voltage = 0;
    for(int i=0; i<samples; i++)
    {
        Turbidity_Sensor_Voltage += ((float)analogRead(TBDPIN)/4880)*5;
    }
    
  Turbidity_Sensor_Voltage = Turbidity_Sensor_Voltage/samples; 
  Turbidity_Sensor_Voltage = round_to_dp(Turbidity_Sensor_Voltage,2);
  if(Turbidity_Sensor_Voltage < 2.5){
    ntu = 3000;
  }
  else{
    ntu = -1120.4*sq(Turbidity_Sensor_Voltage)+ 5742.3*Turbidity_Sensor_Voltage - 4352.9; 
  }

  Serial.print(Turbidity_Sensor_Voltage);
  Serial.println(" V");
  Serial.print(ntu,3);
  Serial.println(" NTU");
  delay(1000);

  Serial.println("____________________________________________________________________");
  delay(1000);
  
  //loop
  sensorsuhu = sensors.getTempCByIndex(0),2;
  delay(10);
  sensorph = ("%.2f", phValue);
  delay(10);
  sensortbd = ("%.2f", ntu);
  delay(10);
  
  //display OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SENDER 2");
  display.setCursor(0,10);
  display.print("LoRa packet sent.");
  display.setCursor(0,20);
  display.print("Counter:");
  display.print(counter);  
  display.setCursor(0,30);
  display.print("suhu:");
  display.print(sensorsuhu);     
  display.setCursor(0,40);
  display.print("pH:");
  display.print(sensorph); 
  display.setCursor(0,50);
  display.print("tbd:");
  display.print(sensortbd); 
  display.display();

  //Send LoRa packet to receiver

  String node2message;
  LoRa.beginPacket(); 
  LoRa.print(counter);
  node2message = node2message + sensorsuhu + "," + sensorph + "," + sensortbd + "," + counter;
  sendMessage(node2message,MasterNode,Node2);
  LoRa.endPacket();

  counter++;
  
  delay(300000);

  if (counter == 300){   
  ESP.restart();
  }
}

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}


void sendMessage(String outgoing, byte MasterNode, byte otherNode) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(MasterNode);               // add destination address
  LoRa.write(Node2);                    // add sender address
  LoRa.write(counter);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
}
