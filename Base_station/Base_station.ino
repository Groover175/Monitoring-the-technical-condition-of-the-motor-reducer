#include <SPI.h>
#include <LoRa.h> 
#include "WiFi.h"
#include <Adafruit_Sensor.h>
#include "SSD1306Wire.h"
#include "InfluxArduino.hpp"


#define SCK 5       // GPIO5 - SX1278's SCK
#define MISO 19     // GPIO19 - SX1278's MISO
#define MOSI 27     // GPIO27 - SX1278's MOSI
#define SS 18       // GPIO18 - SX1278's CS
#define RST 14      // GPIO14 - SX1278's RESET
#define DI0 26      // GPIO26 - SX1278's IRQ (interrupt request)
#define BAND 868E6  // 915E6

InfluxArduino influx;
//connection/ database stuff that needs configuring
const char WIFI_NAME[] = "Redmi";
const char WIFI_PASS[] = "123456789";
const char INFLUX_DATABASE[] = "transporter";
const char INFLUX_IP[] = "192.168.1.253";
//username if authorization is enabled.
const char INFLUX_USER[] = "groover";
//password for if authorization is enabled.
const char INFLUX_PASS[] = "1234";
//measurement name for the database.
const char INFLUX_MEASUREMENT[] = "Transporter";

//how frequently to send data, in microseconds
unsigned long DELAY_TIME_US = 5 * 1000 * 1000;
unsigned long i = 0;
//a variable that we gradually increase in the loop
unsigned long count = 0;

const char* ssid = "Redmi";
const char* password = "123456789";
int packetSize;
String  packet;
float TempData;
String stringT;
String stringC;

SSD1306Wire display (0x3c, 4, 15);

void DisplayRun()
{
    display.clear();
    display.setTextAlignment (TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "RSSI "+String(LoRa.packetRssi())+ "  Rx: " + String(packetSize) + " Bytes");
    display.drawString(5, 20, stringT + " °C\n");
    display.drawString(5, 40, stringC + " A");
    display.display ();
}

String getMeasurements() 
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  packetSize = LoRa.parsePacket();
  if (packetSize) 
  {   // received a packet
      Serial.print("Received packet: \n");
      while (LoRa.available()) 
    {    
      packet = LoRa.readString();  // read packet
      stringT = packet.substring(0,5);
      stringC = packet.substring(5,10);
      Serial.print("Temperature is " + stringT + "°C\n");
      Serial.print("Current is " + stringC + " A\n");
      DisplayRun();
    }
  }
  return packet;
}

void setup() {
  pinMode(16, OUTPUT);
  // set GPIO16 low to reset OLED
  digitalWrite(16, LOW); 
  delay (50);
  // while OLED is running, GPIO16 must go high
  digitalWrite(16, HIGH);
  
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi.."); 
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP()); 
  
  Serial.println("LoRa Receiver");
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

   LoRa.receive();
   //display init
   display.init();
   display.flipScreenVertically ();
   display.setFont(ArialMT_Plain_16);
   //third argument port number
   influx.configure(INFLUX_DATABASE,INFLUX_IP);
   Serial.print("Using HTTPS: ");
   Serial.println(influx.isSecure());
  
}

void loop() {
  getMeasurements();
  delay(100);
  i++;
  
  if(i % 50 == 1) {
    //write our variables.
    char tags[32];
    char fields[32];
    count++;

    //write a tag called new_tag
    sprintf(tags, "sensor = sensor1");
    sprintf(fields, "count = %d, temperature = %0.3f", count, stringT);
    sprintf(tags, "sensor = sensor2");
    sprintf(fields, "count = %d, current = %0.3f", count, stringC);
    bool writeSuccessful = influx.write(INFLUX_MEASUREMENT, tags, fields);
    
    if(!writeSuccessful) {
        Serial.print("error: ");
        Serial.println(influx.getResponse());
    }
  }
}
