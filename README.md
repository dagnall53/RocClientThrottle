# RocClientThrottle
A WiFi Rocrail Client throttle 


## A very simple throttle for Rocrail. 
Uses the new Rocrail 'lcprops' command to get the Loco list. This method sends each loc as a separate Mqtt message.

This code allows for up to 126 locos. Above this the code gets unstable. 
If you have >100 locos you should be considering another "proper" throttle!  

## Use
Compile using the Board = "NodeMCU 1.0 (ESP12E Module)" option.  
Then, after the unit has found the wifi router and connected to the MQTT broker the screen will show an image of a loco and the words "press to refresh Loco List". Press the 5 way button "in" (or "right") and the list should populate with your list of locos from Rocrail. 
When in the top level of the menu, Press the button "up" or "down" to select the loco you want to use.
Press "right" (Away from the OLED) and the screen will show the loco name, and "Speed:0 STOP". The throttle uses the Rocrail loco speeds V_min, V_Mid, V_Cru and V_Max as set for the loco in Rocrail, and can select these speeds in forwards and reverse by selecting Up and down. Pressing the button "in" when moving and the speed is set to 0. Pressing the putton in when the sped iIS 0 will send F2 to the loco which on most is a toot. 
From Speed, press "right" to access a screen where Fn 1-8 can be momentarily triggered. This screen also allows the lights to be turned on and off in position F0. 
From the Fn selection screen, press "right" again to scroll to the loco selection screen.

Pressing "Left" at any time will reboot the code. Do not blame me, the switch is hardwired to reset!.

## Getting the Loco List
From ver004, The way the throttle gets its "list" of locos has been changed. You can either press "in", or "right", and the throttle will request the locolist from rocrail. IF the throttle sees a loco with a name it has seen already, it assumes it has now seen the whole list and will not add any more locos to its internal list. 
A bug/feature of this is that if another throttle (including rocrail) sends any speed coommands to a loco BEFORE the throttle sends its locolist response, then the throttle will see a SINGLE loco and assume that this is the complete loco list. This can be useful to set a throttle to a single loco for a beginner, but resetting the throttle and then pressing in or right (befoore aanyone else commands a speed change) will once again allow the throttle to control ANY loco.   

## Speeds
Because of the simplicity of the throttle, I decided to use the Rocrail preset speeds, so the slowest the loco can move is the first step, going in 4 stages to V-Max. All these settings will be set per loco and so VMax for one loco will be different from another. So the first acceleration will take the loco to V-Min, followed by V-Mid etc. This is similar in principle to how the automatic control in Rocrail works. 


## Hardware
Designed for "WeMos Battery OLED Board". This includes battery, 5 way switch and Oled.

## Rocrail Version
Needs version after: 13870 2018-04-17 07:47:28 +0200 model: extended the lcprops command to itterate all

## Notes
To get xbm images working with the <SSD1306Wire.h>  //https://github.com/ThingPulse/esp8266-oled-ssd1306 it is essential to download and use GIMP https://www.gimp.org/downloads/ Load your image, then export as XBM. None of the simpler LCD image tools correctly format for the XBM format and all I tested had the bit order wrong for the code. Thanks to Jan Vanderborden for directing me in the direction of a solution! 

I am not connected to Rocrail, I just like it and use it.

Rocrail is  Copyright © 2002-2018 Robert Jan Versluis, Rocrail.net. All rights reserved.

