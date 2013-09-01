// (c) Copyright 2013 MCQN Ltd.
// Released under Apache License, version 2.0
//
// Simple example to show how to feed events into choir.io
//
// Usage:
// Wire a button between pin 2 (kButtonPin) and GND
// Update kPath to your choir.io notification KEY
// When the button is pressed, the event will be sent
//
// Requires the HttpClient library from https://github.com/amcewen/HttpClient

#include <SPI.h>
#include <HttpClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>

// Name of the server we want to connect to
const char kHostname[] = "api.choir.io";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/KEY";

// Pin that is wired to a button, and then to ground
const int kButtonPin = 2;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Text strings that we'll send in the HTTP POST request
const char kSoundString[] = "sound=";
const char kLabelString[] = "&label=";
const char kTextString[] = "&text=";

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void setup()
{
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  Serial.println("Setting up network connection");
  
  // Set up the button
  pinMode(kButtonPin, INPUT);
  // Enable the pull-up resistor so we don't need to add one manually
  digitalWrite(kButtonPin, HIGH);

  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(15000);
  }  
  Serial.println("Ready to start notifying");
}

void loop()
{
  int err =0;

  if (digitalRead(kButtonPin) == LOW)
  {
    // Button has been pressed
    err = notifyChoir("g/3", "arduino-test", "Physical button pressed!");
  }
  
  delay(10);
}

int notifyChoir(const char* aSound, const char* aLabel, const char* aText)
{
  int err =0;
  EthernetClient c;
  HttpClient http(c);

  http.beginRequest();  
  err = http.post(kHostname, kPath);
  if (err == 0)
  {
    Serial.println("startedRequest ok");
    // Tell choir.io what format the data is in    
    http.sendHeader("Content-Type", "application/x-www-form-urlencoded");
    // Work out how much data we're going to send
    int contentLen = strlen(kSoundString);
    contentLen += strlen(aSound);
    if (aLabel) {
      contentLen += strlen(kLabelString);
      contentLen += strlen(aLabel);
    }
    if (aText) {
      contentLen += strlen(kTextString);
      contentLen += strlen(aText);
    }
    // Tell choir.io how much data we're sending
    http.sendHeader("Content-Length", contentLen);
    // Now send the data
    http.print(kSoundString);
    http.print(aSound);
    if (aLabel) {
      http.print(kLabelString);
      http.print(aLabel);
    }
    if (aText) {
      http.print(kTextString);
      http.print(aText);
    }
    http.endRequest();

    err = http.responseStatusCode();
    if ((err >= 200) && (err < 300))
    {
      Serial.println("Notified successfully");
    }
    else if (err == 400)
    {
      Serial.println("Error: Input data format incorrect");
    }
    else if (err == 404)
    {
      Serial.println("Error: Key not found");
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();

  return err;
}


