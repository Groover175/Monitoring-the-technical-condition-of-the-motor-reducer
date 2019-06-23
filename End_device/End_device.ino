
#include <SPI.h>
#include <LoRa.h>
#include "OneWire.h"
#include "DallasTemperature.h"

const int currentPin = 36;  // Port with current sensor connected
int sensitivity = 185;      // Sensitivity of current sensor
int adcValue= 0;            // Incoming ADC value
int offsetVoltage = 1650;   // Offset voltage is half the power
double adcVoltage = 0;      // Voltage after analog value transfer
double currentValue = 0;    // Current converted from analog signal

OneWire oneWire(22);
DallasTemperature tempSensor(&oneWire);

// Update delay in microseconds, currently 15-secs (1/4 of minute)
const unsigned long UpdateInterval = 0.1 * (60L * 1000000L); 

void setup() {
  Serial.begin(115200);
  tempSensor.begin(); 
  Serial.println("LoRa Sender starting...");
  LoRa.setPins(18, 14, 26); // LoRa.setPins(CS, RESET, IRQ); 
  
  if (!LoRa.begin(868E6)) { // Set frequency to 433, 868 or 915MHz
    Serial.println("Could not find a valid LoRa transceiver, check pins used and wiring!");
    delay(300);
  }
  Serial.println("Sending data packet...");
  Send_and_Display_Sensor_Data();
  start_sleep();  
}

void start_sleep(){
  esp_sleep_enable_timer_wakeup(UpdateInterval);
  // Set pin-5 to an input as sometimes PIN-5 is used for SPI-SS
  pinMode(BUILTIN_LED,INPUT);
  // In case it's on, turn LED off,      
  // as sometimes PIN-5 on some boards is used 
  // for SPI-SS and can be left low
  digitalWrite(BUILTIN_LED,HIGH); 
  Serial.println("Starting deep-sleep period... awake for " + String(millis()) + "mS");
  // Enough time for the serial port to finish at 115,200 baud
  delay(8);
  esp_deep_sleep_start();   // Sleep for the prescribed time
}

void loop() {
}

void Send_and_Display_Sensor_Data(){

  adcValue = analogRead(currentPin);
  adcVoltage = (adcValue / 4096.0 ) * 3300;
  currentValue = ((adcVoltage - offsetVoltage) / sensitivity);
  
  tempSensor.requestTemperaturesByIndex(0);
  LoRa.beginPacket();       // Start LoRa transceiver
  // Send temperature reading
  LoRa.print(String(tempSensor.getTempCByIndex(0)));
  // Send current reading
  LoRa.print(String(currentValue));
  LoRa.endPacket();         // Confirm end of LoRa data packet
  LoRa.sleep();             // Send LoRa transceiver to sleep
  Serial.print("Temperature : " + String(tempSensor.getTempCByIndex(0)) + "°C\n");
  Serial.print("Current : " + String(currentValue) + " A\n");  
}
