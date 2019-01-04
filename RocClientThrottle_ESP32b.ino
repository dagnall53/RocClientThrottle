#define ver 9
#include "Secrets.h"
#define Rotary_Switch  // comment this out if not using the additional Rotary_Switch switch 

#include <SSD1306.h> // alias for `#include "SSD1306Wire.h"` //https://github.com/ThingPulse/esp8266-oled-ssd1306
// connect to display using pins D1, D2 for I2C on address 0x3c with esp32, 5,4 with esp32?
SSD1306  display(0x3c, OLED_SDA, OLED_SCL);


#include "Terrier.h"

#include <ArduinoOTA.h>
#ifdef _Use_Wifi_Manager
       #include <WiFiManager.h>
#else
       String wifiSSID = SSID_RR;
       String wifiPassword = PASS_RR; 
#endif
String NameOfThisThrottle=ThrottleName;
#ifdef _ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
uint8_t wifiaddr;
uint8_t ip0;
uint8_t ip1;
uint8_t    subIPH;
uint8_t    subIPL;
//uint16_t HandControlID;

// put these changes in PubSubClient.h at or around line 17 or so..(also commented at MQTT.cpp line 9)
//        #define MQTT_MAX_PACKET_SIZE 10000   // lclist is LONG...!
//(And, if using RSMB and not Mosquitto..)
//        #define MQTT_VERSION MQTT_VERSION_3_1 //Rocrail needs to use V 3_1 not 3_1_1 (??)

/* Encoder by Pul Stoffregen 
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 */
#ifdef Rotary_Switch
//#define ENCODER_DO_NOT_USE_INTERRUPTS

 #include <Rotary.h>
 Rotary r= Rotary(EncoderPinA, EncoderPinB);  //D3 D4
 long ThrottlePosition;
 long LastThrottlePosition;
 
 bool Encoder_Timeout;
 bool FunctionUpdated;
 long Encoder_TimeoutAt;
#endif
 long StartupAt;
 bool LcpropsSent;
 // for serial io
 const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;

//LOCO throttle settings from mqtt
extern int LocoNumbers; //set in parse to actual number of locos in lc list


#include "MQTT.h" //new place for mqtt stuff (defines maxlocos
extern String LOCO_id[MAXLOCOS+1];
#include "Menu.h" //place for menu and display stuff, uses loco_id


extern String SW_id[MAXLOCOS+1];
extern int SW_bus[MAXLOCOS+1];
extern int SW_addr[MAXLOCOS+1];



IPAddress ipBroad;
IPAddress mosquitto;
int connects;
bool  WIFI_SETUP;
char rx_byte;  // to try serial input
  int SerioLevel;

 

 
int y;
bool EncoderA = 0;
bool EncoderB = 0;
bool RightButtonState = 0;         // variable for reading the pushbutton status
bool SelectButtonState = 0;         // variable for reading the pushbutton status
bool UpButtonState = 0;         // variable for reading the pushbutton status
bool DownButtonState = 0;         // variable for reading the pushbutton status
bool LeftButtonState = 0;         // variable for reading the pushbutton status
bool buttonpressed;  
uint32_t ButtonPressTimer;
// display and selection stuff
int MenuLevel;
int switchindex;
int RightIndexPos;
int LeftIndexPos;
int locoindex;
int fnindex;
int speedindex;
bool directionindex;
bool AllDataRead;

const int MaxAttribSize = 35;
char FoundAttrib[MaxAttribSize];// FoundAttrib= will be the attributes found by the attrib and attribcolon  functions;
char DebugMessage[128];

#include <EEPROM.h>
#define _EEPROMdebug // adds serial debug helpers
int BrokerAddr;  
void writeString(char add,String data);  // defned here to allow use later but before its defined..
String read_String(char add);             // defned here to allow use later but before its defined..
int ssidEEPROMLocation = 10;
int passwordEEPROMLocation = 100;
int BrokerEEPROMLocation = 1;


void OLED_Display(char* L1,char* L2, char *L3){
  display.clear();
 // Picture();
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, L1);
  display.drawString(64, 24, L2);
  display.drawString(64, 38, L3);
  display.display();
}

void ConnectionPrint() {
  
 char MsgTemp[127];
 int cx;
  Serial.println("");
  Serial.print (F("---------- Connected to SSID:"));
  Serial.print(WiFi.SSID());
  Serial.print(F("  IP:"));
  Serial.println(WiFi.localIP());
 }

void Banner(){
   String MSGText;
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  Picture();
  MSGText="Looking for <";MSGText+=wifiSSID;MSGText+="> ";
  display.drawString(64, 32, MSGText);
  MSGText="code <Ver:";MSGText+=ver;MSGText+="> ";
  display.drawString(64,48,MSGText);display.display();
  
  Serial.println(F("-----------------------------------------------------------"));
  Serial.println(F("             Rocrail Client Throttle    ")); 
    Serial.print(F("-------------------- limit "));
  Serial.print(MAXLOCOS);
  Serial.println(F( " locos -----------------------"));
    Serial.print(F(  "                    revision:"));
  Serial.println(ver);
  Serial.println(F("-----------------------------------------------------------"));
  delay(100);
}


void TrySerial_IO(){
  String MSGText;
    String TestData;
    long Timestarted;
    bool UpdateInProgress;
    // will try to change wifiSSID,wifiPassword
    UpdateInProgress=false;
    Timestarted=millis();
    Picture();display.drawString(64, 32, "Pause for Serial IO"); display.display();
    Serial.println("");
    Serial.println(F("--- To enter new wifi SSID / Password type 'xxx' BEFORE wifi connects--- "));
    Serial.println(F("-- Use 'Newline' OR 'CR' to end input line, BUT NOT BOTH together --"));
 while ((millis()<= Timestarted+3000) || UpdateInProgress) {
    delay(1); // Allow esp to process other events .. may not be needed, but here to be safe..                                      
    recvWithEndMarker();
    if (newData == true) {TestData=receivedChars;
                          //Serial.print("<");Serial.print(TestData);Serial.print("> Looking for {");Serial.print(LookFor);Serial.println("}");
                          switch (SerioLevel){ 
                          case 0:
                                 if (TestData=="xxx\0"){
                                    UpdateInProgress=true;
                                    display.clear(); display.drawString(64, 32, "Type in New SSID"); display.display();
                                    Serial.println("-");
                                    Serial.println("--Update the SSID Started--");
                                    Serial.print("  SSID currently is <");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                    Serial.println("Type in New SSID");newData = false;SerioLevel=1;
                                     }
                          break;
                          case 1:  wifiSSID=receivedChars; newData = false;SerioLevel=2;
                          MSGText="SSID     <";
                          MSGText+=wifiSSID;
                          MSGText+=">";
                                   display.clear(); display.drawString(64, 12, MSGText); display.display();
                                   Serial.print(" SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                   Serial.println("Type in New Password");
                                    
                          break;

                          case 2:   
                                 wifiPassword=receivedChars; newData = false;SerioLevel=3;
                                 MSGText="Password <";
                                 MSGText+=wifiPassword;
                                 MSGText+=">";
                                   display.drawString(64, 24, MSGText); display.display();
                                 Serial.print(" SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                 Serial.println("Type in MQTT Broker address");
                                    
                          break;
                          case 3:
                                 BrokerAddr= TestData.toInt(); newData = false;SerioLevel=4;
                                 MSGText="Broker Addr<";
                                 MSGText+=BrokerAddr;
                                 MSGText+=">";
                                 display.drawString(64, 36, MSGText); display.display();
                                 Serial.print("Broker Addr:");Serial.print(BrokerAddr);Serial.print(" WiFi SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                 Serial.println("Type sss to save, rrr to return to start");
                                
                                
                          case 4:
                              Serial.print("Broker addr:");Serial.print(BrokerAddr);Serial.print(" WiFi SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                              if (TestData=="sss\0"){
                                    display.clear(); display.drawString(64, 32, "EEPROM Updated"); display.display();
                                    Serial.println("I will now save this data and continue");
                                    
                                        //write to eeprom here
                                        writeString(ssidEEPROMLocation,wifiSSID);delay(10);
                                        writeString(passwordEEPROMLocation,wifiPassword);delay(10);
                                        EEPROM.write(BrokerEEPROMLocation,BrokerAddr);delay(10);
                                        EEPROM.commit();delay(100);
                                        //
                                        
                                    UpdateInProgress=false;
                                    newData = false;SerioLevel=5;
                                    }
                             else {
                              if (TestData=="rrr\0"){ display.drawString(64, 46, "Type xxx to resume input"); display.display();Serial.println("-----------------");Serial.println("---Starting again---");Serial.println(" Type xxx again to re-start sequence");
                                  newData = false;SerioLevel=0;
                                 }else{Serial.println("Please type 'sss' to save, or 'rrr' to return to start");newData = false;}
                                  }
                          
                          break;

                          case 5:
                            newData = false;
                            Serial.println("Please turn off to restart with new values");

                          break;
                          default:
                            newData = false;
                            Serial.println("Not Understood");  
                          break; 
                         }
                          
                            }
      showNewData();  // only shows results if newData=true  and not understood (can happen if looping after a rrr)
} 
}

void ConnectToWifi(String WiSSID, String Password, int Broker){
//Void Status(){
String MSGText;
  delay(100);
  #ifdef _Use_Wifi_Manager
     WiFiManager wifiManager;  // this  stores ssid and password invisibly  !!
    //reset settings - for testing
    //wifiManager.resetSettings();
    wifiManager.autoConnect("ROCNODE ESP AP");  //ap name (with no password) to be used if last ssid password not found
  #else    

    WiFi.mode(WIFI_STA);  //Alternate "normal" connection to wifi
    #ifdef _ESP32
      // nothing needed here, the ESP32 doe not have the 'set output power' function
    #else
      WiFi.setOutputPower(30);//  0 sets transmit power to 0dbm to lower power consumption, but reduces usable range.. try 30 for extra range
    #endif
   MSGText="connect to SSID<";  MSGText+=  WiSSID;  MSGText+="> ";
   Picture();display.drawString(64, 32, MSGText); display.display();
   MSGText+=" password <";  MSGText+=  Password;  MSGText+="> "; 
   Serial.print ("- Trying to ");Serial.print(MSGText);Serial.println(" --");
   WiFi.begin(WiSSID.c_str(), Password.c_str());
   delay(100);
   while (WiFi.status() != WL_CONNECTED) {delay(100);Serial.print("."); }
 
 #endif // not using wifi manager

        //if you get here you should have connected to the WiFi
        //  Serial.println();Serial.println(F("--Connected--"));
 
   ipBroad = WiFi.localIP();
   subIPH = ipBroad[2];
   subIPL = ipBroad[3];
   wifiaddr = ipBroad[3];
   ConnectionPrint();
  //   ++++++++++ MQTT setup stuff   +++++++++++++++++++++
   mosquitto[0] = ipBroad[0]; mosquitto[1] = ipBroad[1]; mosquitto[2] = ipBroad[2];
   mosquitto[3] = Broker; // mosquitto address 
   Serial.print(F("--------Mosquitto will first try to connect to:"));
   Serial.println(mosquitto);
   MQTT_Setup();  
   //if you get here you should have connected to the MQTT broker but reconnect() in MQTT includes code to search addr 3 to 50 if its not at the expected address
    Serial.println(F("----------------MQTT NOW setup ----------------")); 
    WIFI_SETUP=true;
  _SetupOTA(ThrottleName); // now we  have set the ota update with nickname ThrottleName
 
  Picture(); display.drawString(64, 32, "WiFi Connected"); display.display();delay(1000);
  char MsgTemp[127];
  int cx;
  cx= sprintf (MsgTemp, " IP: %d:%d:%d:%d ", ipBroad[0],ipBroad[1],ipBroad[2],wifiaddr);
  Picture(); display.drawString(64, 32, MsgTemp);
  display.display();  //display.setFont(ArialMT_Plain_16);
  delay(1000); 
}




void PrintLocoSettings(){
        for (int loco=1; loco<= LocoNumbers; loco++){
            Serial.print(F("Attributes stored for <"));Serial.print(loco);Serial.print(F("> <"));Serial.print(LOCO_id[loco]);
         
      Serial.println("");}
}

void Picture(){
  display.clear(); 
  display.drawXbm(1,1, Terrier_Logo_width, Terrier_Logo_height, Terrier_Logo); 
 //  display.display();
}
void _SetupOTA(String StuffToAdd){
  String Name;
  // ota stuff  Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]


 Name="RocMouse<";
 Name=Name+StuffToAdd;
 Name=Name+">";
 Serial.printf("------ Setting OTA Hostname <%s> -------------\n",Name.c_str());
 Serial.printf("------------------------------------------------------\n");
 ArduinoOTA.setHostname(Name.c_str());
  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  //---------------------end ota stuff -------
   
}

void setup() {
  // init serial port 
  EEPROM.begin(512); //Initialize EEPROM
  Serial.begin(115200);
  delay(500); // allow time for serial to set up
  Serial.println("-----Serial Port Running------"); 
  // set the builtin LED pin to work as an output
  pinMode(LED_BUILTIN, OUTPUT);
  WIFI_SETUP=false;
  BrokerAddr=BrokerAddrDefault;
  wifiSSID=read_String(ssidEEPROMLocation);
  wifiPassword=read_String(passwordEEPROMLocation);
  BrokerAddr=EEPROM.read(BrokerEEPROMLocation);

  if (wifiSSID==""){wifiSSID=SSID_RR;}         // if empty, try the secrets default
  if (wifiPassword==""){wifiPassword=PASS_RR;} // if empty, try the secrets default
  if (BrokerAddr==0){BrokerAddr=BrokerAddrDefault;}   // zero is not a valid Ip for the broker
  
 pinMode(RightButton, INPUT_PULLUP);
 pinMode(SelectButton, INPUT_PULLUP);
 pinMode(UpButton, INPUT_PULLUP);
 pinMode(DownButton, INPUT_PULLUP);
 pinMode(LeftButton, INPUT_PULLUP);
 //pinMode(D8, INPUT_PULLUP);  // test
   
 
  // init the display
  Banner();
  TrySerial_IO();
  connects=0;
  ConnectToWifi(wifiSSID,wifiPassword,BrokerAddr);
  //Status();  // where we try to log in to WiFi

  #ifdef Rotary_Switch
      pinMode(EncoderPinA, INPUT_PULLUP);   // Rotary_Switch encoder pins
      pinMode(EncoderPinB, INPUT_PULLUP); 
      //ThumbWheel.write(0);
      ThrottlePosition  = 0;
      LastThrottlePosition=0;
    
      FunctionUpdated=true;
   #endif

  buttonpressed=false;
  MenuLevel=0;
  switchindex=1;
  locoindex=0;
  fnindex=0;
  RightIndexPos=3;
  LeftIndexPos=0;
  directionindex=true;
  LocoNumbers=0; 
  AllDataRead=false; 
  for (int i=0; i<=(MAXLOCOS) ;i++) {
    LOCO_id[i]="~blank~";
    }
  StartupAt=millis();
  LcpropsSent=false; 

  GetLocoList();
}

void GetLocoList(){
  ParseIndex=0;
  AllDataRead=false;
  MQTT_DO(); // needed to make the function work in startup, before mqtt is running in the main loop
  MQTTSend("rocrail/service/client","<model cmd=\"lcprops\" />");
}

void GetLocoFunctions(int index){
  char MsgTemp[127];
  int cx;
  cx= sprintf ( MsgTemp,"<model cmd=\"lcprops\" val=\"%s\"/>",LOCO_id[index].c_str());
  //Serial.println(MsgTemp);  //https://wiki.rocrail.net/doku.php?id=cs-protocol-en
  
  MQTTSend("rocrail/service/client",MsgTemp);

}

 void MQTT_DO(void){
    // MQTT stuff, & check we are connected.. 
  if (!MQTT_Connected()) {reconnect();}           // was if (!client.connected()) 
      MQTT_Loop(); // for client.loop(); //gets wifi messages etc..
 }
// serial io testing here




void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    while (Serial.available() > 0 && newData == false) {
           rc = Serial.read();
           if ((rc != 10)&&(rc != 13) ) // look for end with  NL or CR
                {  
                        receivedChars[ndx] = rc;
                        ndx++; if (ndx >= numChars) { ndx = numChars - 1; } //truncate 
                }else {
                       newData = true;
                       if (ndx>>0){receivedChars[ndx] = '\0';} // terminate the string only if we have actually got something (stops LF AND CR immediately entering a null string)
                       ndx = 0; //ready for next data ..
                       }
    }
}

void showNewData() {
    if (newData == true) {
                Serial.println(" ");
                Serial.print("Seen but not understood <");
                Serial.print(receivedChars);Serial.println(">    ");
              // to assist debug serial io //Serial.print("Ascii is <"); for (int i=0; i<=(numChars) ;i++) { Serial.print(int(receivedChars[i]));Serial.print("> <");}
                newData = false;}
    }




void Do_RotarySW(){
    long ThrottlePos;
    unsigned char result=r.process();
     if (result == DIR_NONE) {
    // do nothing
                             }
    else if (result == DIR_CW) {
       if(!Encoder_Timeout){ Encoder_Timeout=true; Encoder_TimeoutAt=millis();ThrottlePos=ThrottlePos+1;
       Serial.print("<"); FunctionUpdated=false;ButtonUp(MenuLevel);LastThrottlePosition=ThrottlePosition;
                           }
                                }
  else if (result == DIR_CCW) {
       if(!Encoder_Timeout){ Encoder_Timeout=true; Encoder_TimeoutAt=millis();ThrottlePos=ThrottlePos-1;
       Serial.print(">"); FunctionUpdated=false;ButtonDown(MenuLevel);LastThrottlePosition=ThrottlePosition;
                           }
}
}

 
void loop() {
   recvWithEndMarker();
   showNewData();

//  EncoderA= digitalRead(EncoderPinA);
//  EncoderB= digitalRead(EncoderPinB); 
//  Serial.print("Encoder Buttons A<");Serial.print(EncoderA);Serial.print("  B<");Serial.print(EncoderB);Serial.println("> ");
//  delay(200);  // for debug
    
    RightButtonState = digitalRead(RightButton);
    SelectButtonState = digitalRead(SelectButton);
    UpButtonState = digitalRead(UpButton);
    DownButtonState = digitalRead(DownButton);
    LeftButtonState = digitalRead(LeftButton); //new 
  //delay(100);Serial.print(" Buttons r<");Serial.print(RightButtonState);Serial.print("  s<");Serial.print(SelectButtonState);Serial.print(">  u<");Serial.print(UpButtonState);Serial.print(">  d<");Serial.print(DownButtonState);Serial.println(">");

  #ifdef Rotary_Switch
   Do_RotarySW();
    if ((millis()-Encoder_TimeoutAt)>=50){Encoder_Timeout=false;} // sets max click repetition rate for encoder
  #endif 
 
  display.clear();  
  MQTT_DO();   
  
  DoDisplay(MenuLevel);


  // turn off the LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1);// essentially this is the main loop delay


  //Serial.print(" Buttons 3<");Serial.print(RightButtonState);Serial.print("  4<");Serial.print(buttonState4);Serial.print("  5<");Serial.print(SelectButtonState);Serial.print(">  6<");Serial.print(UpButtonState);Serial.print(">  7<");Serial.print(DownButtonState);Serial.println(">");
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (!buttonpressed&&!RightButtonState) {
    buttonpressed=true;ButtonPressTimer=millis();
   ;ButtonRight(MenuLevel);
          MenuLevel=MenuLevel+1; // nb cannot change MenuLevel in a function that called with menulevel as a variable,
          if (LocoNumbers>=2){if (MenuLevel>= 3){MenuLevel=0;}}
           else {if (MenuLevel>= 3){MenuLevel=1;}
                }
           }
  if (!buttonpressed&&!SelectButtonState) {
    Serial.print("s");
    if (MenuLevel==0){GetLocoFunctions(locoindex); MenuLevel=MenuLevel+1;} // selected loco, change to speed [level 1] display
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonSelect(MenuLevel);     
      } 
  if (!buttonpressed&&!UpButtonState) {
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonUp(MenuLevel);
    } 
  if (!buttonpressed&&!DownButtonState) {
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonDown(MenuLevel);
    } 
   if (!buttonpressed){
    ButtonInactive(MenuLevel); 
   }
 //  TO Be debugged  if (!buttonpressed&&!LeftButtonState) {
 //   buttonpressed=true;ButtonPressTimer=millis();
 //   ButtonLeft(MenuLevel);
 //   MenuLevel=MenuLevel-1; // nb cannot change MenuLevel in a function that called with menulevel as a variable,
 //          if (MenuLevel<=0){MenuLevel=3;}
 //   } 
    
    #ifdef Rotary_Switch
      if (millis()-ButtonPressTimer>=500) {buttonpressed=false;}// sets relatively slow repetition rate for push buttons
    #endif
    #ifndef Rotary_Switch
     if (millis()-ButtonPressTimer>=200) {buttonpressed=false;}// sets faster repetition rate if no Rotary_Switch sw present push buttons
    #endif 


  ArduinoOTA.handle();

 // turn on the LED
 digitalWrite(LED_BUILTIN, LOW);
 // delay(1);

}
void writeString(char add,String data)
{
  int _size = data.length();
  int i;
#ifdef _EEPROMdebug
       Serial.print("Writing :");Serial.print(_size);Serial.print(" bytes of data <");Serial.print(data);Serial.print("> at EEprom addr:");Serial.println(int(add));
#endif
  for(i=0;i<_size;i++) { EEPROM.write(add+i,data[i]); }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
}
 
 
String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
    while(k != '\0' && len<510)   //Read until null character, assume EEPROM 512 or thereabouts
  {    
    k=EEPROM.read(add+len);
    data[len]=k; 
    len++;
  }
  data[len]='\0';
  #ifdef _EEPROMdebug
  Serial.print("Read EEprom Data at Addr:");Serial.print(int(add));Serial.print(" was<");Serial.print(data);Serial.println("> ");
  #endif
    return String(data);
}

