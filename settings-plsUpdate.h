/*
  Copyright (c) 2014 Kamesh Raghavendra

  This program notifies changes in ambient temperature beyond a
  set threshold through a Twitter feed
*/

/* Temboo account keys */
#define TEMBOO_ACCOUNT ""
#define TEMBOO_APP_KEY_NAME ""
#define TEMBOO_APP_KEY  ""

/* Twitter account keys */
#define TEMBOO_TWITTER_PROFILE ""
#define TWITTER_ACCESS_TOKEN ""
#define TWITTER_ACCESS_TOKEN_SECRET ""
#define TWITTER_API_KEY ""
#define TWITTER_API_SECRET ""

/* InfluxDB account keys */
#define INFLUXDB_SERVER ""
#define INFLUXDB_PORT ""
#define INFLUXDB_USER ""
#define INFLUXDB_PASSWORD ""
#define INFLUXDB_DATABASE ""
#define DBSERIES_NAME ""

/* Arduino resources */
#define TEMPERATURE_PIN 0 //temperature sensor PIN number
#define TEMP_SENSITIVITY 0.5 //in centigrade
#define POLL_WINDOW 10000 //in milli-secs

/* OpenWRT/SD resources - create the directory structure & copy the rules engine */
#define ARDUINAL_DIR "/mnt/sd/arduinal"
#define LOG_FILENAME "/mnt/sd/arduinal/logs/arduinal-log.txt"
#define RULES_FILENAME "/mnt/sd/arduinal/rules-engine/arduinal.py"
