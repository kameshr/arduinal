########################################################################
# Copyright (c) 2014 Kamesh Raghavendra
#
# This program is a rules-engine to be run in the OpenWRT environment
# for intelligent rule-based processing of data readings
# 
# Requires Arduino Yun, OpenWRT 1.3+, SD card, temperature sensor
#
# Python dependies:
#	1. Python 2.7 or later
#	2. influxdb client (pip install influxdb)
#	3. TwitterAPI client (pip install TwitterAPI)
#	4. Updated settings.py to be placed in the same dir on SD card
#
#######################################################################

from influxdb import client as influxdb
import settings
from TwitterAPI import TwitterAPI
import time
import sys

def logMessage(message):
	print message

def tweetMessage(message):
	api = TwitterAPI(settings.TWITTER_API_KEY, settings.TWITTER_API_SECRET,
			settings.TWITTER_ACCESS_TOKEN, settings.TWITTER_ACCESS_TOKEN_SECRET
			)
	r = api.request('statuses/update', {'status':message})
	logMessage(r.status_code)

def queryDB(queryString):
	db = influxdb.InfluxDBClient(settings.INFLUXDB_SERVER, settings.INFLUXDB_PORT,
			settings.INFLUXDB_USER, settings.INFLUXDB_PASSWORD, settings.INFLUXDB_DATABASE
			)
	result = db.query(queryString)
	return result

def logTemperature(seriesName, tempC):
	db = influxdb.InfluxDBClient(settings.INFLUXDB_SERVER, settings.INFLUXDB_PORT,
			settings.INFLUXDB_USER, settings.INFLUXDB_PASSWORD, settings.INFLUXDB_DATABASE
			)
	timeObject = time.gmtime(None)
	data = [
			{"points": [ [timeObject[0], timeObject[1], timeObject[2], timeObject[3], timeObject[4],
				timeObject[5], timeObject[6], timeObject[7], "UTC", tempC] ],
			"name": seriesName,
			"columns": ["Year", "Month", "Date", "Hour", "Minute",
				"Second", "WeekDay", "YearDay", "TimeZone", "Temperature"]
			}
	]
	db.write_points(data)

if len(sys.argv) > 1 and sys.argv[1] == 'logTemperature':
	if len(sys.argv) > 2 and isinstance(sys.argv[2], basestring):
		if len(sys.argv) > 3 and isinstance(float(sys.argv[3]), float):
			logTemperature(sys.argv[2], float(sys.argv[3]))
		else:
			logMessage("ERROR: Expected float value | Usage: arduinal.py logTemperature <series-name> <temperature value>")
	else:
		logMessage("ERROR: Wrong series name | Usage: arduinal.py logTemperature <series-name> <temperature value>")
else:
	logMessage("ERROR: Wrong command | Usage: arduinal.py logTemperature <series-name> <temperature value>")
