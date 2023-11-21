/*
 Controlling LED/GPIO using Bluetooth
 http:://www.electronicwings.com
*/ 

#include "BluetoothSerial.h"
#include <TinyGPS++.h>
#define GPS_BAUDRATE 9600 

TinyGPSPlus gps;

const char LED= 2;
const char turnON ='a';
const char turnOFF ='b';

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
 Serial.begin(9600);
 Serial2.begin(GPS_BAUDRATE);
 pinMode(LED, OUTPUT);
 SerialBT.begin("ESP32test"); //Bluetooth device name
 Serial.println(F("ESP32 - GPS module"));
 Serial.println("The device started, now you can pair it with bluetooth!");
 Serial.println("Now You can TURN ON LED by sending 'a' and TURN OFF by 'b'");
}

void loop() {
 char message;
 
 if (SerialBT.available()) {
    message=SerialBT.read();
    Serial.write(message);
   if(message==turnON){
      Serial.println(" :LED Turned ON");
      SerialBT.println("LED Turned ON");   
      digitalWrite(LED, HIGH);      //Turn LED ON
    while(1){ 
      message=SerialBT.read();
      Serial.write(message);
      if (Serial2.available() > 0) {
        if (gps.encode(Serial2.read())) {
          if (gps.location.isValid()) {
            SerialBT.print(F("- latitude: "));
            SerialBT.println(gps.location.lat());

            SerialBT.print(F("- longitude: "));
            SerialBT.println(gps.location.lng());

            SerialBT.print(F("- altitude: "));
            if (gps.altitude.isValid())
              SerialBT.println(gps.altitude.meters());
            else
              SerialBT.println(F("INVALID"));
          } else {
            SerialBT.println(F("- location: INVALID"));
          }

          SerialBT.print(F("- speed: "));
          if (gps.speed.isValid()) {
            SerialBT.print(gps.speed.kmph());
            SerialBT.println(F(" km/h"));
          } else {
            SerialBT.println(F("INVALID"));
          }

          SerialBT.print(F("- GPS date&time: "));
          if (gps.date.isValid() && gps.time.isValid()) {
            SerialBT.print(gps.date.year());
            SerialBT.print(F("-"));
            SerialBT.print(gps.date.month());
            SerialBT.print(F("-"));
            SerialBT.print(gps.date.day());
            SerialBT.print(F(" "));
            SerialBT.print(gps.time.hour());
            SerialBT.print(F(":"));
            SerialBT.print(gps.time.minute());
            SerialBT.print(F(":"));
            SerialBT.println(gps.time.second());
          } else {
            SerialBT.println(F("INVALID"));
          }

          SerialBT.println();
        }
      }

      if (millis() > 5000 && gps.charsProcessed() < 10)
        SerialBT.println(F("No GPS data received: check wiring"));

      if(message==turnOFF){
        break;
      }
    }
   }

 
   else{
     Serial.println(" :Invalid Input"); 
     SerialBT.println("Invalid Input");
   } 
 }
 delay(20);
}