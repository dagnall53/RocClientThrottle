# RocClientThrottle
A WiFi Rocrail Client throttle 

## A Simple throttle for Rocrail. 
Uses the new Rocrail 'lcprops' command to get the Loco list. This method sends each command as a separate MQTT message.
Now optimised to use a rotary switch and pushbutton. 

## Compiling
Compatible with ESP32 and ESP8266 hardware. 
Tested with Arduino Board = "NodeMCU 1.0 (ESP12E Module)" and Board= '"Wemos" Wifi+Bluetooth'  options.
Uses these Arduino Libraries, which must be installed for the program to compile:
<Rotary.h>    //https://github.com/brianlow/Rotary
<JC_Button.h> // https://github.com/JChristensen/JC_Button
<SSD1306.h>   // https://github.com/ThingPulse/esp8266-oled-ssd1306
<PubSubClient.h>  //https://github.com/knolleary/pubsubclient
Because Rocrail sends unusually long MQTT messages, it is important to increase the size of message that PubSubClient can accept. To do this you need to moodify your PubSubClient.h at or around line 17 or so..to add  #define MQTT_MAX_PACKET_SIZE 10000  

## USE
When the unit is turned on, it waits 3 seconds to allow you to modify its WiFi router settings etc. After this short delay it will connect to the wifi, find the Rocrail MQTt broker, and ask Rocrail for a list of Locos. It will then display a screen that allows you to select which loco to control. This list only shows locos that are Active and Not hidden in the Rocrail Loco table. 

Whilst on the "Select Loco" menu, you can LONGPress the select button to change the Rocrail Power State. LongPress for more than 1 second will turn Power OFF. Pressing and holding for longer than two seconds will turn power back ON. An indicator in the top left of the screen shows the current RocRail power status.

A quick, short press of the select button moves from Select Loco to Speed Control. A Longpress here will instantly take the selected loco to Speed=0. Moving the rotary switch or selecting up or down buttons will change speed and send this command to RocRail.

A quick, short press of the select button moves from Speed Control to Functions. Up and down (or rotate the rotary switch) select which function to operate. A LongPress will then send the function command to Rocrail. If the Function is a momentary function, the Throttle will send a "start", and rocrail will later turn the function off at the time set in the loco function table. If the Function is a toggle, like for example "lights", the throttle will toggle between the two states with every "Longpress". If you have set names for the functions, the throttle will display these names.

A quick, short press of the select button then moves from Functions back to Select Loco.

## Hardware
The code is designed by default to use the NodeMCU / Wemos OLED + 18650 battery +4 way switch board. 
With this board, Press "right" (Away from the OLED) to scroll through the menu levels, Pressing the button "in" is used for the short and long presses. 
Pressing "Left" at any time on this hardware will reboot the code. Do not blame me, the switch is hardwired to reset!. 
The code also works nicely with the WiFi Bluetooth Battery Esp32 0.96 Inch OLED board, but you will have to add connections to an external rotary switch. 
(All connections needed for the OLED, Rotary Switch and 4 way or 5 way switches are listed in the Secrets.h file).


## Select Loco 
Up and Down buttons (or the Rotary switch) can be used to select which loco to control. 
A small display at the bottom of the screen shoows how many locos are available.

## Speed Control 
The Throttle should pick up any speed changes made by Rocrail. If you change the speed, Rocrail will change the Loco Speed. 
If you LongPress the select button, the speed will immediately be dropped to 0.
The Rotary switch uses knowledge of the V_Max set in the rocrail loco table to modify how the speed changes. This should give about one and a half turns from 0 to full speed. 
Up and down buttons add or subtract 5 to the speed.

## Functions 
When change to the function view, the throttle requests Function settings from Rocrail and uses these to display the Function names, and to decide if the Function is momentary or toggled. Function states should be mirrored if any changes are made by other throttles, but may not be immediately in synch until some changes have been made.

## Setting WiFi name, password, etc.
When first turned on, the throttle waits 3 seconds to allow you to enter "xxx" via a serial port. 
This makes the throttle enter a mode where you can change the saved WIFI name and password, and other settings.
If you have connected the Throttle to a terminal emulator set to speed 115200 baud, you should see text like this as the throttle turns on:  
"           This is Rocrail Client Throttle 
"            Named      "ESP8266Throttle" 
"--- To enter new wifi SSID / Password type 'xxx' BEFORE wifi connects--- 
"-- Use 'Newline' OR 'CR' to end input line  --
 
If you then press "xxx" (+enter with lf or cr) on a connected terminal,  the Throttle will prompt sequentially for a new Router name, password, MQTT broker address and nickname for the throttle. Pressing (return) will keep the current settings, but typing anything will change the settings. 
The last menu otion is to press "sss" or "rrr". sss(+cr or lf) will save the new entries to the EEPROM, and the throttle will start. "rrr" will go to the start of the entry process to allow you to reenter any changes.  



## Rocrail Version
Needs version after: 13870 2018-04-17 07:47:28 +0200 model: extended the lcprops command to itterate all

## Notes
To get xbm images working with the <SSD1306Wire.h>  //https://github.com/ThingPulse/esp8266-oled-ssd1306 it is essential to download and use GIMP https://www.gimp.org/downloads/ Load your image, then export as XBM. None of the simpler LCD image tools correctly format for the XBM format and all I tested had the bit order wrong for the code. Thanks to Jan Vanderborden for directing me in the direction of a solution! 

Ass noted at the start.. Its very important to increase the size that the MQTT interface can use: 
// put this in pubsubclient.h in your arduino/libraries/PubSubClint/src 
#define MQTT_MAX_PACKET_SIZE 10000   // lclist is LONG...!


I am not connected to Rocrail, I just like it and use it.

Rocrail is  Copyright © 2002-2018 Robert Jan Versluis, Rocrail.net. All rights reserved.

