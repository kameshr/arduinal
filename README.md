Copyright (c) 2014 Kamesh Raghavendra

This application observes ambient temperature & intelligently notifies anomalies

Requires Arduino Yun, OpenWRT 1.3+, SD card, temperature sensor

There are two components to this application:
sketches: This runs in the ATmega32u4 side of the bridge, reports temperature changes
rules-engine: This runs in the AR9331 OpenWRT Linux side of the bridge, detects anomalies in temperature

Refer to INTALL for installation intructions

See sample Twitter notifications here: https://twitter.com/arduinal
