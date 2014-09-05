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

/* Global variables */
float tempC = 0;
float tweetTemp = 0;
int tempPin = 0;
float tempThresh = 2; //threshold of temperature change for notifying
int event = 1; //number of notification events
char* logFileName = "/mnt/sda1/logs/ArduinalTemp.txt"; //Needs an SD card on Arduino Yun
int pollWindow = 10000; //in millisecs

void setup() {  
  Bridge.begin();
  FileSystem.begin();
}

void loop()
{ 
  tempC = analogRead(tempPin);
  tempC = (5.0*tempC*100.0)/1024.0;
  
  if(abs(tempC - tweetTemp) >= tempThresh) {
    String relative;
    if(tempC > tweetTemp) {
      relative = "increased";
    } else {
      relative = "decreased";
    }
      
    String tweetText("Event[" + (String)event + "]: Temperature " + relative + " to " + String(tempC) + " degC.");
    
    unsigned int returnCode = tweetMessage(tweetText);

    if (returnCode == 0) {
        tweetTemp = tempC;
        event++;
    }
  }
  delay(10000);
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
  
  if (returnCode == 0) {
        logFile.println("Tweeted: " + message);
    } else {
      while (StatusesUpdateChoreo.available()) {
        char c = StatusesUpdateChoreo.read();
        logFile.print(c);
      }
    }
    StatusesUpdateChoreo.close();
    logFile.close();
    return returnCode;
}
