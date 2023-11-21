/*
 Controlling LED/GPIO using Bluetooth
 http:://www.electronicwings.com
*/ 

#include "BluetoothSerial.h"
#include <TinyGPS++.h>
#define GPS_BAUDRATE 9600 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SDA_2 33
#define SCL_2 32
#define SDA_1 21
#define SCL_1 22

TinyGPSPlus gps;
Adafruit_BMP280 bmp;
Adafruit_MPU6050 mpu;

const char LED= 2;
const char turnON ='a';
const char turnOFF ='b';

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void setup() {
 Serial.begin(9600);
 Serial2.begin(GPS_BAUDRATE);
 Wire.begin(SDA_2, SCL_2);
 Wire1.begin(SDA_1, SCL_1);
 bmp.begin(0x76);
   if (!mpu.begin(0x68,&Wire1)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
 pinMode(LED, OUTPUT);
 SerialBT.begin("ESP32test"); //Bluetooth device name
 Serial.println(F("ESP32 - GPS module"));
 Serial.println("The device started, now you can pair it with bluetooth!");
 Serial.println("Now You can TURN ON LED by sending 'a' and TURN OFF by 'b'");
 bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

}

void loop() {
 char message;
 
 if (SerialBT.available()) {
    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
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
        SerialBT.print(F("Temperature = "));
        SerialBT.print(bmp.readTemperature());
        SerialBT.println(" *C");

        SerialBT.print(F("Pressure = "));
        SerialBT.print(bmp.readPressure());
        SerialBT.println(" Pa");

        SerialBT.print(F("Approx altitude = "));
        SerialBT.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
        SerialBT.println(" m");

        SerialBT.println("--------------------");

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        /* Print out the values */
        SerialBT.print("Acceleration X: ");
        SerialBT.print(a.acceleration.x);
        SerialBT.print(", Y: ");
        SerialBT.print(a.acceleration.y);
        SerialBT.print(", Z: ");
        SerialBT.print(a.acceleration.z);
        SerialBT.println(" m/s^2");

        SerialBT.print("Rotation X: ");
        SerialBT.print(g.gyro.x);
        SerialBT.print(", Y: ");
        SerialBT.print(g.gyro.y);
        SerialBT.print(", Z: ");
        SerialBT.print(g.gyro.z);
        SerialBT.println(" rad/s");

        SerialBT.print("Temperature: ");
        SerialBT.print(temp.temperature);
        SerialBT.println(" degC");

        SerialBT.println("");

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

      delay(1000);
    }
   }

 
   else{
     Serial.println(" :Invalid Input"); 
     SerialBT.println("Invalid Input");
   } 
 }
 delay(20);
}