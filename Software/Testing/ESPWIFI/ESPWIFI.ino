#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>

// Set these to your desired credentials.
const char *ssid = SSID;
const char *password = PASS;

NetworkServer server(80);

#include <NMEAGPS.h>

#define TXD2 17
#define RXD2 18
#define GPS_PORT Serial2

static NMEAGPS gps;
static gps_fix fix;
String data = "";

// STORAGE
#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// PROGRAM START

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Failing");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial2.begin(38400, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Server started");

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  appendFile(LittleFS, "/files/data.txt", "DATA START\n");
  appendFile(LittleFS, "/files/data.txt", "lat,lon,time,satellites,altitude,date,heading,speed\n");
}

void loop() {
  NetworkClient client = server.accept();

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            //client.print("Click <a href=\"/H\">here</a><br>");
            client.print(data);
            data = "";

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /DATA")) {
          
        }/*
        if (currentLine.endsWith("GET /L")) {
          Serial.println("L");
        }*/
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }

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
    String d = "Data:" + lat + "," + lon + "," + time + "," + String(satellites) + "," + String(altitude, 4) + "," + date + "," + String(heading, 4) + "," + String(speed, 4) + "\n";
    appendFile(LittleFS, "/files/data.txt", d.c_str());
    if (data.length() <= 5000) {
      data += d;
    }
  }
  
  if (Serial.available()) {
    String input = Serial.readString();
    Serial.println(input);
    if (input == "Read\n") {
      readFile(LittleFS, "/files/data.txt");
    } else if (input == "Reset\n") {
      writeFile(LittleFS, "/files/data.txt", "DATA START\n");
      appendFile(LittleFS, "/files/data.txt", "lat,lon,time,satellites,altitude,date,heading,speed\n");
    }
  }
}