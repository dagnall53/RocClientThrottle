# RocClientThrottle
A WiFi Rocrail Client throttle 
WITH ESP32 compatibility


## A very simple throttle for Rocrail. 
Uses the new Rocrail 'lcprops' command to get the Loco list. This method sends each loc as a separate Mqtt message.

This code is preset for 40 locos. Too many locos and the code may get unstable. 
If you have lots of locos you should probably be considering a "proper" throttle!, or you can change the #define MAXLOCOS 40 in MQTT.h line 7 

## First Setup
The hardware now (v 12 on) uses EEPROM for SSID, password Broker addr and Throttle name.
You can change these by entering "xxx"(+new line) in a terminal connected to the Throttle COM port and set at 115200 baud within 3 seconds or so of the board start up. A prompt:  

"--- To enter new wifi SSID / Password type 'xxx' BEFORE wifi connects---"

"-- Use 'Newline' OR 'CR' to end input line, BUT NOT BOTH together --"

appears in the terminal window and  

"Pause for Serial IO"

appears on the OLED.

If the EEPROM data is not valid (i.e. you have not changed the default SSID in secrets.h) the hardware will force you to enter data before it attempts to log into the router.. 

## Use
Compile using the Board = "NodeMCU 1.0 (ESP12E Module)" option for ESP8266  Or "Wemos" Wifi+Bluetooth" for ESP32. Other board selections may work, but have not been tested.
Then, after the unit has the EEPRO updated via serial (use the Arduino Serial monitor) and has found the wifi router and connected to the MQTT broker the screen will show a list populated with your list of locos from Rocrail. This only shows locos that are "active" and not "Hidden" in the Rocrail locomotives table.  
When in the top level of the menu, Press the button "up" or "down" (or use the rotary switch) to select the loco you want to use.
Press "right" (Away from the OLED) and the screen will show the loco name, and "Speed:". Speed should update from Rocrail. Pressing the button "in" when moving and the speed is immediately set to 0 {STOP}. Pressing the putton in when the speed IS 0 will send F2 to the loco which on most locos is a toot. 
From Speed, press "right" to access a screen where the functions can be set/checked . This screen also allows the lights to be turned on and off in position F0. From V006 the throttle picks up the function names defined in the Rocrail Loco table. 
From the Fn selection screen, press "right" again to scroll to the loco selection screen.

Pressing "Left" at any time on the esp8266 oled board will reboot the code. Do not blame me, the switch is hardwired to reset!. but I have added some untested "ButtonLeft" code intended to be used with D8 but this is currently UNTESTED.


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
Also works with Wemos ESP32 "wifi+bluetooth +battery" board. But you need to add your own switches

Starting with Version 8, most of the hardware wiring is now defined in Secrets.h This was necessary since the ESP32 uses different pinouts in places. 

## Rocrail Version
Needs version after: 13870 2018-04-17 07:47:28 +0200 model: extended the lcprops command to itterate all

## Notes
Uses: 
  * SSD 1306 OLED library (#include "SSD1306Wire.h") ` //https://github.com/ThingPulse/esp8266-oled-ssd1306
      To get xbm images working with the <SSD1306Wire.h>  //https://github.com/ThingPulse/esp8266-oled-ssd1306 it is essential to download and use GIMP https://www.gimp.org/downloads/ Load your image, then export as XBM. None of the simpler LCD image tools correctly format for the XBM format and all I tested had the bit order wrong for the code. Thanks to Jan Vanderborden for directing me in the direction of a solution! 

  * Rotary Encoder (#include <Rotary.h>) https://github.com/brianlow/Rotary
  * MQQT interface (#include <PubSubClient.h>) https://github.com/knolleary/pubsubclient  ...Its very important to increase the size that the MQTT interface can use:  // SO.. put this modification in pubsubclient.h in your arduino/libraries/PubSubClint/src (somewhere like line 26 )
  
     #define MQTT_MAX_PACKET_SIZE 10000   // Because Rocrails lclist is very LONG for a MQQT message...!


I am not connected to Rocrail, I just like it and use it.

Rocrail is  Copyright © 2002-2018 Robert Jan Versluis, Rocrail.net. All rights reserved.

