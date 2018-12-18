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

Pressing "Select" OR "Right" both now trigger the Lcprops message, so its easier to get the loco list (My 5 way switches are sometimes hard to "select")
There is a new check for previously seen locos, BUT IF the throttle sees a lc message loco with a name it has seen already, it immediately assumes it has now seen the whole list and will not add any more locos to its internal list. 

This has some "interesting" consequences..Remember that any rocrail speed change triggers a lc message that can be mistaken for a LCprops response, and you will see that when any throttle (including rocrail) sends speed commands to a loco, the wiireless throttle sees these commands and if it has not got a complete list of locos, will try to add the loco name (but note the speed command does not include Vmax etc or function name parameters) to its internal list of available locos. 
So, when a SECOND speed command is sent to the same loco, then the wireless throttle code will assume that it has now seen the whole loco list, and it will not allow any more items to be added to its internal loco list. 

This could be useful to set a throttle to a single loco for a beginner (but resetting the throttle and then pressing in or right (before anyone else commands a speed change) will once again allow the throttle to control ANY loco).  BUT note that if you are not using my #rotary define, the code will try to use the Rocrail Vmin, Vcruise,Vmax etc setting, BUT will not have see the real properties, so these will be set to zero, so speed changes will not work. (Function commands to send sounds etc should work). Again, this mmight be a nice feature for beginners, but I doubt it!

You can now also "cycle" the menu levels around and providing you do not change the loco selection, the speed will remain as set. 
If you do change the loco selection, then the speed will set to zero to prevent mishaps.   

## Speeds
Because of the simplicity of the throttle, I decided to use the Rocrail preset speeds if the rotary switch is not used. So the slowest the loco can move is the first Vmin step, going up 4 stages to V-Max. All these settings will be set per loco and so VMax for one loco will be different from another. So the first acceleration will take the loco to V-Min, followed by V-Mid etc. This is similar in principle to how the automatic control in Rocrail works.
If you are using the rotary switch version, speeds are changed in "1" steps by the rotary control and "10" steps by the up and down buttons. If you are at speed zero, pressing the select (in) button sounds F1. If you are moving, pressing the same button acts as an emergency brake and sets speed zero.


## Hardware
Designed for "WeMos Battery OLED Board". This includes battery, 5 way switch and Oled.

## Rocrail Version
Needs version after: 13870 2018-04-17 07:47:28 +0200 model: extended the lcprops command to itterate all

## Notes
To get xbm images working with the <SSD1306Wire.h>  //https://github.com/ThingPulse/esp8266-oled-ssd1306 it is essential to download and use GIMP https://www.gimp.org/downloads/ Load your image, then export as XBM. None of the simpler LCD image tools correctly format for the XBM format and all I tested had the bit order wrong for the code. Thanks to Jan Vanderborden for directing me in the direction of a solution! 

Its very important to increase the size that the MQTT interface can use: 
// put this in pubsubclient.h in your arduino/libraries/PubSubClint/src 
#define MQTT_MAX_PACKET_SIZE 10000   // lclist is LONG...!

I am not connected to Rocrail, I just like it and use it.

Rocrail is  Copyright © 2002-2018 Robert Jan Versluis, Rocrail.net. All rights reserved.

