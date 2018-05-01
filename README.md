# RocClientThrottle
A WiFi Rocrail Client throttle 


## Improved version of octothrottle. 
Uses the new Rocrail 'lcprops' command to get the Loco list. This method sends each loc as a separate Mqtt message, so does not suffer the Message Limit that Octo Throttle had. 

This code allows for up to 126 locos. Above this the code gets unstable. If you have >100 locos you should be considering another "proper" throttle!  

## Hardware
Designed for "WeMos Battery OLED Board". This includes battery, 5 way switch and Oled.

## Rocrail Version
Needs version after: 13870 2018-04-17 07:47:28 +0200 model: extended the lcprops command to itterate all

## Notes
To get xbm images working with the <SSD1306Wire.h>  //https://github.com/ThingPulse/esp8266-oled-ssd1306 it is essential to download and use GIMP https://www.gimp.org/downloads/ Load your image, then export as XBM. None of the simpler LCD image tools correctly format for the XBM format and all I tested had the bit order wrong for the code. Thanks to Jan Vanderborden for directing me in the direction of a solution! 

I am not connected to Rocrail, I just like it and use it.

Rocrail is  Copyright © 2002-2018 Robert Jan Versluis, Rocrail.net. All rights reserved.

