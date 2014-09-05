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
float tweetTemp = 0;
float logTemp = 0;

/* Global settings */
int tempPin = 0;
float tweetThresh = 5; //threshold of temperature change for notifying
float logThresh = 0.5; //threshold of temperature change for logging
char* logFileName = "/mnt/sda1/logs/ArduinalTemp.txt"; //Needs an SD card on Arduino Yun
int pollWindow = 10000; //in millisecs

void setup() {  
  Bridge.begin();
  FileSystem.begin();
}

void loop()
{ 
  /* Probe temperature */
  tempC = analogRead(tempPin);
  tempC = (5.0*tempC*100.0)/1024.0;
  
  if(abs(tempC - logTemp) >= logThresh) {
    logTemperature(tempC);
    logTemp = tempC;
  }
  
  /* Poll for possible notification events */
  if(abs(tempC - tweetTemp) >= tweetThresh) {
    String relative;
    if(tempC > tweetTemp) {
      relative = "increased";
    } else {
      relative = "decreased";
    }
    
    /* Fetch the date-time string from OpenWRT kernel */
    String timeValue = getTime();
    
    /* Construct message text and dispatch notifications */
    String tweetText("Event[" + timeValue + "]: Temperature " + relative + " to " + String(tempC) + " degC.");
    tweetMessage(tweetText);
    tweetTemp = tempC;
  }
  delay(pollWindow);
}

/* Return the current time as a space de-limited string */
String getTime() {
  Process date;
  String value = "";

/* Fetch the date-time string from OpenWRT kernel */
  if(!date.running()) {
    date.begin("date");
    date.run();
  }
  
  while(date.available() > 0) {
    value = date.readString();
    value.trim();
  }
  
/* //Code to convert time string into CSV format
  value.replace("UTC ", "");
  value.replace("  ", ",");
  value.replace(" ", ",");
  value.replace(":", ",");
  value = "\"" + timeString + "\"";
*/

  return value;
}

/* Tweet function, currently uses Temboo libraries */
unsigned int tweetMessage(String message) {
  TembooChoreo StatusesUpdateChoreo;
  unsigned int returnCode;
  File logFile = FileSystem.open(logFileName, FILE_APPEND);
  while(!logFile);
  StatusesUpdateChoreo.begin();
  StatusesUpdateChoreo.setAccountName(TEMBOO_ACCOUNT);
  StatusesUpdateChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  StatusesUpdateChoreo.setAppKey(TEMBOO_APP_KEY);
  StatusesUpdateChoreo.setChoreo("/Library/Twitter/Tweets/StatusesUpdate");

  StatusesUpdateChoreo.addInput("AccessToken", TWITTER_ACCESS_TOKEN);
  StatusesUpdateChoreo.addInput("AccessTokenSecret", TWITTER_ACCESS_TOKEN_SECRET);
  StatusesUpdateChoreo.addInput("ConsumerKey", TWITTER_API_KEY);    
  StatusesUpdateChoreo.addInput("ConsumerSecret", TWITTER_API_SECRET);
  StatusesUpdateChoreo.addInput("StatusUpdate", message);
  
  returnCode = StatusesUpdateChoreo.run();
  
  if (returnCode != 0) {
      while (StatusesUpdateChoreo.available()) {
        String c = StatusesUpdateChoreo.readString();
        logMessage(c);
      }
    }
    StatusesUpdateChoreo.close();
    return returnCode;
}

/* Logging function, uses a local log file on SD card */
unsigned int logMessage(String message) {
  File logFile = FileSystem.open(logFileName, FILE_APPEND);
  while(!logFile);
  logFile.println(message);
  logFile.close();
  return 0;
}

/* Logging function, uses a local log file on SD card */
unsigned int logTemperature(float temp) {
  Process restLog;

  String dataPost = "[{\"name\":\"time-series\",\"columns\":[\"Temperature\"],\"points\":[ [\"" + (String)tempC + "\"]]}]";
  String dbURL = String(INFLUXDB_SERVER) + ":" + String(INFLUXDB_PORT) + "/db/" + String(INFLUXDB_DATABASE) + "/series?u=" + String(INFLUXDB_USER) + "&p=" + String(INFLUXDB_PASSWORD);
  String cmdString = String("curl -X POST -d \'" + dataPost + "\' \'" + dbURL + "\' ");

  restLog.runShellCommand(cmdString);
  while(restLog.running());
  
  return 0;
}
