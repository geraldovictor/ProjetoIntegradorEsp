/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-gps
 */

#include <TinyGPS++.h>
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define GPS_BAUDRATE 9600 // The default baudrate of NEO-6M is 9600

char charVal[10]; 
TinyGPSPlus gps; // the TinyGPS++ object
BluetoothSerial SerialBT;
const char turnON = 'a';
const char turnOFF = 'b';

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void setup()
{
  Serial.begin(9600);
  Serial2.begin(GPS_BAUDRATE);
  Serial.println("$PMTK220,200*2C");
  Serial.println(F("ESP32 - GPS module"));
  SerialBT.begin("ESP32test");

  if (!SD.begin(5))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
}

void loop()
{
  char message;

  if (SerialBT.available())
  {
    message = SerialBT.read();
    Serial.write(message);
    if (message == turnON)
    {
      writeFile(SD, "/teste.txt", "");
      while (1)
      {
        message = SerialBT.read();
        Serial.write(message);
        if (Serial2.available() > 0)
        {

          if (gps.encode(Serial2.read()))
          {
            if (gps.location.isValid() && gps.altitude.isValid())
            {
              SerialBT.print(gps.location.lat());
              dtostrf(gps.location.lat(), 4, 3, charVal); //escrita no arquivo
              appendFile(SD, "/teste.txt", charVal);
              SerialBT.print(",");
              appendFile(SD, "/teste.txt", ",");
              SerialBT.println(gps.location.lng());
              dtostrf(gps.location.lng(), 4, 3, charVal); 
              appendFile(SD, "/teste.txt", charVal); // prints no terminal bt
              SerialBT.print(",");
              appendFile(SD, "/teste.txt", ",");
              SerialBT.println(gps.altitude.meters());
              dtostrf(gps.altitude.meters(), 4, 3, charVal);
              appendFile(SD, "/teste.txt", charVal);
              appendFile(SD, "/teste.txt", "\n");
            }
            else
            {
              SerialBT.println("Locatizacao ou altitude inválidas");
            }
          }
        }

        if (millis() > 5000 && gps.charsProcessed() < 10)
        {
          SerialBT.println("No GPS data received: check wiring");
          appendFile(SD, "/teste.txt", "No GPS data received: check wiring");
        }

        if (message == turnOFF)
        {
          break;
        }
      }
    }

    else
    {
      Serial.println(" :Invalid Input");
      SerialBT.println("Invalid Input");
    }
  }
}
