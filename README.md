# RocClientThrottle
A WiFi Rocrail Client throttle 


## Improved version of octothrottle. 
Uses the new Rocrail 'lcprops' command to get the Loco list. This method sends each loc as a separate Mqtt message, so does not suffer the Message Limit that Octo Throttle had. 

This code allows for up to 126 locos. Above this the code gets unstable.

## Hardware
Designed for "WeMos Battery OLED Board". This includes battery, 5 way switch and Oled.

## Rocrail Version
Needs version after: 13870 2018-04-17 07:47:28 +0200 model: extended the lcprops command to itterate all

## Notes
I am not connected to Rocrail, I just use it.

Rocrail is  Copyright © 2002-2018 Robert Jan Versluis, Rocrail.net. All rights reserved.

