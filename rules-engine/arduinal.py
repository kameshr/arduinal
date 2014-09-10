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

# Log a message to a local file on Arduino SD card
# uses setting LOG_FILENAME defined in settings.py
def logMessage(message):
	logFile = open(settings.LOG_FILENAME, 'a')
	logFile.write('[' + time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.gmtime()) + '] ' + str(message) + '\n')
	logFile.close()

# Broadcast a notification on Twitter, uses APIs defined in settings.py
def tweetMessage(message):
	api = TwitterAPI(settings.TWITTER_API_KEY, settings.TWITTER_API_SECRET,
			settings.TWITTER_ACCESS_TOKEN, settings.TWITTER_ACCESS_TOKEN_SECRET
			)
	r = api.request('statuses/update', {'status':message})
	logMessage('Tweeted message [' + message + '] with status ' + r.status_code)

# Query time-series data in the cloud, DB settings defined in settings.py
def queryDB(queryString):
	db = influxdb.InfluxDBClient(settings.INFLUXDB_SERVER, settings.INFLUXDB_PORT,
			settings.INFLUXDB_USER, settings.INFLUXDB_PASSWORD, settings.INFLUXDB_DATABASE
			)
	result = db.query(queryString)
	return result

# Log a temperature value from the sensor, kick start detection of anomalies
# Script calls from across the bridge get routed here
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

# Build necessary state variables from current/historical data
# into the global state data-structure
def buildState():
	settings.state = dict()
	settings.state['update_time'] = time.gmtime()
	queryString = str('select mean(Temperature), stddev(Temperature), count(Temperature) from time-series group by Hour')
	results = queryDB(queryString)
	for i in results[0]['points'][:]:
		settings.state[str(i[4]) + '_hr'] = {results[0]['columns'][1]: i[1], results[0]['columns'][2]: i[2], results[0]['columns'][3]: i[3]}

# Validate the currency of the state data-structures
def validateState():
	if settings.state.has_key('update_time'):
		current_time = time.gmtime()
		if current_time[0] != setings.state['update_time'][0] or current_time[7] != setings.state['update_time'][7]:
			buildState()
	else:
		buildState()

# Main script that routes requests from across the bridge
# and notifies anomalies detected
if len(sys.argv) > 1 and sys.argv[1] == 'logTemperature':
	if len(sys.argv) > 2 and isinstance(sys.argv[2], basestring):
		if len(sys.argv) > 3 and isinstance(float(sys.argv[3]), float):
			logTemperature(sys.argv[2], float(sys.argv[3]))
		else:
			logMessage("ERROR: Expected float value | Usage: arduinal.py logTemperature <series-name> <temperature value>")
	else:
		logMessage("ERROR: Wrong series name | Usage: arduinal.py logTemperature <series-name> <temperature value>")
elif len(sys.argv) > 1 and sys.argv[1] == 'test-debug':
	validateState()
	print settings.state
else:
	logMessage("ERROR: Wrong command | Usage: arduinal.py logTemperature <series-name> <temperature value>")
