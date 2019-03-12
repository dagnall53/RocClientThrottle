#ifndef NVSettingInterface_h
 #define NVSettingInterface_h

 // the #include statment and code go here...
 #include "Arduino.h"
 #include <EEPROM.h>
 // #define _EEPROMdebug // adds serial debug help 
 const byte numChars = 32; //max size I will allow the saved ssid, password and throttle name 
 const int BrokerEEPROMLocation =1;
 const int ssidEEPROMLocation = 10;
 const int passwordEEPROMLocation = ssidEEPROMLocation+numChars;
 const int ThrottleNameEEPROMLocation = passwordEEPROMLocation+numChars;
 const int CalEEPROMLocation=ThrottleNameEEPROMLocation+numChars;
 const int EndofEEprom = CalEEPROMLocation+10; // some spare room
 const int EEPROM_Size = EndofEEprom+1; 

 
 int MSG_content_length();
 void CheckForSerialInput();
 void recvWithEndMarker(); // Gets data <receivedChars> from serial interface
 void showNewData() ;    // shows recieved (serial) <receivedChars> data on serial terminal, useful for debugging 
 void WriteWiFiSettings();
 void TestFillEEPROM(int d);
 void writeString(int add,String data);  // defned here to allow use later but before its properly defined..
 String read_String(int add);             // defned here to allow use later but before its defined..
 void recvWithEndMarker() ;
 void showNewData();



#endif
