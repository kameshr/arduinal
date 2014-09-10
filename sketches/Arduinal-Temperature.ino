/*
  Copyright (c) 2014 Kamesh Raghavendra

  This program notifies changes in ambient temperature beyond a
  set threshold through a Twitter feed
  Requires Arduino Yun, OpenWRT 1.3+, SD card, temperature sensor
*/

#include <Bridge.h>
#include <Temboo.h>
#include <Process.h>
#include <FileIO.h>
#include "Accounts.h"

/* Global state variables */
float tempC = 0;
float logTemp = 0;

void setup() {
  Bridge.begin();
  FileSystem.begin();
}

void loop()
{ 
  /* Probe temperature */
  tempC = analogRead(TEMPERATURE_PIN);
  tempC = (5.0*tempC*100.0)/1024.0;
  
  if(abs(tempC - logTemp) >= TEMP_SENSITIVITY) {
    logTemperature(tempC);
    logTemp = tempC;
  }
  delay(POLL_WINDOW);
}

/* Logging function, uses a local log file on SD card */
unsigned int logMessage(String message) {
  File logFile = FileSystem.open(LOG_FILENAME, FILE_APPEND);
  while(!logFile);
  logFile.println(message);
  logFile.close();
  return 0;
}

/* Pass temperature to a local rules-engine */
unsigned int logTemperature(float temp) {
  Process restLog;
  String value;
  
  restLog.runShellCommand("python " + String(RULES_FILENAME) + " logTemperature " + String(DBSERIES_NAME) + " " + (String)tempC);
  while(restLog.running());
  
  while(restLog.available() > 0) {
    value = restLog.readString();
    value.trim();
    logMessage(value);
  }
  
  return 0;
}
