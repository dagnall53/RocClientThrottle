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

You can now also "cycle" the menu levels around and providing you do not change the loco selection, the speed will remain as set. 
If you do change the loco selection, then the speed will set to zero to prevent mishaps.   

When you "select" a Loco and change to the speed view, the throttle requests Function settigs from Rocrail and uses these to display the Function names, and to decide if the Function is momentary or toggled. Function states should be mirrored if any changes are made by other throttles, but may not be immediately in synch until some changes have been made.

## Speeds
Because of the simplicity of the throttle, from V006 the step speed changes have been removed. 

The rotary switch changes speeds in "1" steps by the rotary control and "10" steps by the up and down buttons. If you are at speed zero, pressing the select (in) button sounds F2. (F2 is 'Toot' on all my locos) {you can change this in Menu.h, line 357} If you are moving, pressing the same button acts as an emergency brake and sets speed zero.


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

