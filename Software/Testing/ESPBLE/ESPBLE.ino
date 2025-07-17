#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLESecurity.h>

#include <NMEAGPS.h>
#include "pass.h"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLESecurity *pSecurity = new BLESecurity();
bool deviceConnected = false;

#define TXD2 17
#define RXD2 18
#define GPS_PORT Serial2

static NMEAGPS gps;
static gps_fix fix;
String data;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Disconnected BLE");
    pServer->startAdvertising();
  }
};

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  Serial.println("Starting BLE");
  BLEDevice::init("ESP32-Raph");
  // Create the server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the service ESP_LE_AUTH_BOND
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  
  // Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->startAdvertising();

  // Set security to none
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  
  Serial2.begin(38400, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Setup finished");
}

// the loop function runs over and over again forever
void loop() {
  // GPS
  while (gps.available(Serial2)) {
    fix = gps.read();
    String lat = String(fix.latitudeL());
    String lon = String(fix.longitudeL());
    if (!lat.equals("0")) {
      lat = lat.substring(0, lat.length() - 7) + "." + lat.substring(lat.length() - 7);
    }
    if (!lon.equals("0")) {
      lon = lon.substring(0, lon.length() - 7) + "." + lon.substring(lon.length() - 7);
    }
    String time = String(fix.dateTime.hours);
    if (fix.dateTime.minutes < 10) {
      time += "0" + String(fix.dateTime.minutes);
    } else {
      time += String(fix.dateTime.minutes);
    }
    if (fix.dateTime.seconds < 10) {
      time += "0" + String(fix.dateTime.seconds);
    } else {
      time += String(fix.dateTime.seconds);
    }
    uint8_t satellites = fix.satellites;
    double altitude = (double) fix.altitude_cm() / 100.0;
    String date = String(fix.dateTime.date);
    if (fix.dateTime.month < 10) {
      date += "0" + String(fix.dateTime.month);
    } else {
      date += String(fix.dateTime.month);
    }
    if (fix.dateTime.year < 10) {
      date += "0" + String(fix.dateTime.year);
    } else {
      date += String(fix.dateTime.year);
    }
    double heading = (double)fix.heading_cd() / 100.0;
    double speed = (double) fix.speed_mkn() / 1000.0;

    Serial.printf("Data:%s,%s,%s,%d,%f,%s,%f,%f\n", lat, lon, time, satellites, altitude, date, heading, speed);
    if (deviceConnected) {
      pCharacteristic->setValue("Data:" + lat);
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(lon);
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(time);
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(String(satellites));
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(String(altitude, 4));
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(date);
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(String(heading, 4));
      pCharacteristic->notify();
      delay(50);
      pCharacteristic->setValue(String(speed, 4));
      pCharacteristic->notify();
      delay(50);
    }
  }
}
