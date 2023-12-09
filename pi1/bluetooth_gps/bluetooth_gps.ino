/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-gps
 */

/* outros links :
https://www.arduino.cc/reference/en/language/variables/data-types/stringobject/
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
const char turnONVoo1 = '1';
const char turnONVoo2 = '2';
const char turnONVoo3 = '3';
const char turnONVoo4 = '4';
const char turnONVoo5 = '5';
const char turnONVoo6 = '6';
const char turnONVoo7 = '7';
const char turnONVoo8 = '8';
const char turnONVoo9 = '9';
const char turnOFF = 'b';

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    SerialBT.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    SerialBT.println("File written");
  }
  else
  {
    SerialBT.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    SerialBT.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    SerialBT.println("Message appended");
  }
  else
  {
    SerialBT.println("Append failed");
  }
  file.close();
}

void setup()
{
  Serial.begin(9600);
  Serial2.begin(GPS_BAUDRATE);
  SerialBT.println("$PMTK220,200*2C");
  SerialBT.println(F("ESP32 - GPS module"));
  SerialBT.begin("ESP32test");

  if (!SD.begin(5))
  {
    SerialBT.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    SerialBT.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    SerialBT.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    SerialBT.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    SerialBT.println("SDHC");
  }
  else
  {
    SerialBT.println("UNKNOWN");
  }
  writeFile(SD, "/voo1.txt", "");
  writeFile(SD, "/voo2.txt", "");
  writeFile(SD, "/voo3.txt", "");
  writeFile(SD, "/voo4.txt", "");
  writeFile(SD, "/voo5.txt", "");
  writeFile(SD, "/voo6.txt", "");
  writeFile(SD, "/voo7.txt", "");
  writeFile(SD, "/voo8.txt", "");
  writeFile(SD, "/voo9.txt", "");
}

void loop()
{
  char message = turnOFF; 
  char path[10];  // char[10] para armazenar "/vooX.txt" onde X é de 1 a 9

  if (SerialBT.available())
  {
    message = SerialBT.read();
    Serial.write(message);
    if (message != turnOFF){
      snprintf(path, sizeof(path), "/voo%c.txt", message);    //define path = "/vooX.txt" onde X é de 1 a 9
      appendFile(SD, path, "__________\n"); // separa 2 testes feitos em um mesmo arquivo
    }
  }
  while (message != turnOFF)
  {
    if (Serial2.available() > 0)
    {

      if (gps.encode(Serial2.read()))
      {
        if (gps.location.isValid() && gps.altitude.isValid() && gps.speed.isValid())
        {
          dtostrf(millis(), 4, 3, charVal);
          SerialBT.print(charVal); //<- checar se funciona
          SerialBT.print(":"); //<- checar se funciona
          SerialBT.print(gps.location.lat());
          dtostrf(millis(), 4, 3, charVal);
          appendFile(SD, path, charVal); 
          appendFile(SD, path, ":"); 
          dtostrf(gps.location.lat(), 4, 3, charVal); 
          appendFile(SD, path, charVal);
          SerialBT.print(",");
          appendFile(SD, path, ",");
          SerialBT.println(gps.location.lng());
          dtostrf(gps.location.lng(), 4, 3, charVal);
          appendFile(SD, path, charVal); 
          SerialBT.print(",");
          appendFile(SD, path, ",");
          SerialBT.println(gps.altitude.meters());
          dtostrf(gps.altitude.meters(), 4, 3, charVal);
          appendFile(SD, path, charVal);
          SerialBT.print(",");
          appendFile(SD, path, ",");
          SerialBT.print(gps.speed.kmph());
          dtostrf(gps.speed.kmph(), 4, 3, charVal);
          appendFile(SD, path, charVal);
          appendFile(SD, path, "\n");
        }
        else
          SerialBT.println("Locatizacao, altitude ou velocidade inválidas");
      }
    }

    if (millis() > 5000 && gps.charsProcessed() < 10)
      SerialBT.println("No GPS data received: check wiring");

    if (SerialBT.available())
    {
      if(SerialBT.read() == turnOFF)
      {
        message = turnOFF;
        Serial.write(message);
      }
    }
  }
}
