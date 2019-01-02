#define ver 8
#include "Secrets.h"
#define Rotary  // comment this out if not using the additional rotary switch 

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
#ifdef Rotary
#define ENCODER_DO_NOT_USE_INTERRUPTS
 #include <Encoder.h>
 Encoder ThumbWheel(EncoderPinA, EncoderPinB);  //D3 D4
 long ThrottlePosition;
 long LastThrottlePosition;
 long ThrottleClicks;
 bool EncoderMoved;
 bool FunctionUpdated;
 long EncoderMovedAt;
#endif
 long StartupAt;
 bool LcpropsSent;
 

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
  Serial.println(F("---------------------------Connected-----------------------"));
  Serial.print (F(" Connected to SSID:"));
  Serial.print(WiFi.SSID());
  Serial.print(F("  IP:"));
  Serial.println(WiFi.localIP());
 
 
 //cx= sprintf (MsgTemp, " IP: %d:%d:%d:%d ", ipBroad[0],ipBroad[1],ipBroad[2],wifiaddr);
 //OLED_Display("Connected",MsgTemp,"");
 //delay(500); 

 
}


void Status(){
delay(10);
  Serial.println();Serial.println();
  Serial.println(F("-----------------------------------------------------------"));
  Serial.println(F("             ESP8266 Rocrail Client 'Throttle'    ")); 
  Serial.print(F("-------------------- limit "));
  Serial.print(MAXLOCOS);
  Serial.println(F( " locos ------------------------"));
  Serial.print(F(  "                    revision:"));
  Serial.println(ver);
  Serial.println(F("-----------------------------------------------------------"));
#ifdef _ESP32

#else
WiFi.setOutputPower(20); //  0 sets transmit power to 0dbm to lower power consumption, but reduces usable range.. try 30 for extra range
#endif
  

#ifdef _Use_Wifi_Manager
   WiFiManager wifiManager;  // this  stores ssid and password invisibly  !!
  //reset settings - for testing
  //wifiManager.resetSettings();
  wifiManager.autoConnect("ROCNODE ESP AP");  //ap name (with no password) to be used if last ssid password not found
#else    

  WiFi.mode(WIFI_STA);  //Alternate "normal" connection to wifi
#ifdef _ESP32

#else
  WiFi.setOutputPower(30);
#endif

  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  Serial.print(F("Trying to connect to {"));  Serial.print(wifiSSID.c_str());Serial.print(F("} "));
  while (WiFi.status() != WL_CONNECTED) {delay(500);Serial.print(".");}
 
#endif 

//if you get here you have connected to the WiFi
  Serial.println();Serial.println(F("--Connected--"));
 
  ipBroad = WiFi.localIP();
  subIPH = ipBroad[2];
  subIPL = ipBroad[3];
  wifiaddr = ipBroad[3];
  ConnectionPrint();
  ipBroad[3] = 255; //Set broadcast to local broadcast ip e.g. 192.168.0.255 // used in udp version of this program
 
 //   ++++++++++ MQTT setup stuff   +++++++++++++++++++++
  mosquitto[0] = ipBroad[0]; mosquitto[1] = ipBroad[1]; mosquitto[2] = ipBroad[2];
  mosquitto[3] = BrokerAddr; //18;                //forced  mosquitto address, where the broker is! originally saved as RN[14], 
  Serial.print(F(" Mosquitto will first try to connect to:"));
  Serial.println(mosquitto);
  //char Message[80];
  //sprintf(Message, "SUBIP:%i", mosquitto[3]);
  //OLED_Display("MQTT Setup",Message,"");
  //delay(100);
  MQTT_Setup();  

    Serial.println(F("-----------MQTT NOW setup ----------------")); 
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
 Serial.printf("--- Setting OTA Hostname <%s> -------------\n",Name.c_str());
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
 String MSGText;
  Serial.begin(115200);

  // set the builtin LED pin to work as an output
 pinMode(LED_BUILTIN, OUTPUT);
  
 pinMode(RightButton, INPUT_PULLUP);
 pinMode(SelectButton, INPUT_PULLUP);
 pinMode(UpButton, INPUT_PULLUP);
 pinMode(DownButton, INPUT_PULLUP);
 pinMode(LeftButton, INPUT_PULLUP);
 
#ifdef Rotary
pinMode(EncoderPinA, INPUT_PULLUP);   // rotary encoder pins
pinMode(EncoderPinB, INPUT_PULLUP);
#endif
 
  // init the display
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
 
  Picture();
  MSGText="code <Ver:";MSGText+=ver;MSGText+="> ";
  display.drawString(64,48,MSGText);
  display.drawString(64, 32, "Looking for WiFi");display.display();
  delay(1000);

 connects=0;
 Status();
 _SetupOTA(ThrottleName); // now we  have set the ota update nickname ThrottleName
 
 Picture();
  display.drawString(64, 32, "WiFi Connected"); display.display();delay(1000);
  char MsgTemp[127];
  int cx;
  cx= sprintf (MsgTemp, " IP: %d:%d:%d:%d ", ipBroad[0],ipBroad[1],ipBroad[2],wifiaddr);
 Picture();
  display.drawString(64, 32, MsgTemp);
  display.display();  display.setFont(ArialMT_Plain_16);
  delay(1000); 
  Picture();display.display();
  delay(1000);  
// initial defaults
#ifdef Rotary
ThumbWheel.write(0);
ThrottlePosition  = 0;
LastThrottlePosition=0;
ThrottleClicks=-999;
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
 
void loop() {
//EncoderA=digitalRead(EncoderPinA);
//EncoderB= digitalRead(EncoderPinB); 
    RightButtonState = digitalRead(RightButton);
    SelectButtonState = digitalRead(SelectButton);
    UpButtonState = digitalRead(UpButton);
    DownButtonState = digitalRead(DownButton);
    LeftButtonState = digitalRead(LeftButton); //new 
//delay(100);Serial.print(" Buttons r<");Serial.print(RightButtonState);Serial.print("  s<");Serial.print(SelectButtonState);Serial.print(">  u<");Serial.print(UpButtonState);Serial.print(">  d<");Serial.print(DownButtonState);Serial.println(">");
//Serial.print("Encoder Buttons A<");Serial.print(EncoderA);Serial.print("  B<");Serial.print(EncoderB);Serial.println("> ");
//delay(200);  // for debug
  #ifdef Rotary
    long ThrottlePos;
    ThrottlePos = ThumbWheel.read();
//  Serial.print (" throttlepos:"); Serial.println(ThrottlePosition);   // un comment for debug
  if (ThrottlePos != ThrottleClicks ) {
    ThrottleClicks = ThrottlePos;
    ThumbWheel.write(ThrottleClicks);
    ThrottlePosition=int(ThrottleClicks/4);// up in steps of 1?}
        
   //Serial.print("Throttleclicks  = ");Serial.print(ThrottleClicks); // un comment for debug
   
   //Serial.print (" lastthrottlepos:"); Serial.println(LastThrottlePosition);   // un comment for debug
   
    if(!EncoderMoved){    EncoderMoved=true; EncoderMovedAt=millis();
     if ((ThrottlePosition-LastThrottlePosition)>=1){Serial.print("<");
          FunctionUpdated=false;ButtonUp(MenuLevel);LastThrottlePosition=ThrottlePosition;}
     if ((ThrottlePosition-LastThrottlePosition)<=-1){Serial.print(">");
          FunctionUpdated=false;ButtonDown(MenuLevel);LastThrottlePosition=ThrottlePosition;}
    // if (abs(ThrottlePosition-LastThrottlePosition)>=1){FunctionUpdated=false;LastThrottlePosition=ThrottlePosition;}
                }
             }
 //Serial.print("AFTER Rotary Buttons 3<");Serial.print(RightButtonState);Serial.print("  5<");Serial.print(SelectButtonState);Serial.print(">  6<");Serial.print(UpButtonState);Serial.print(">  7<");Serial.print(DownButtonState);Serial.println(">");

    if ((millis()-EncoderMovedAt)>=100){EncoderMoved=false;} // sets max click repetition rate for encoder
 //   if (!FunctionUpdated){SetLoco(locoindex,ThrottlePosition); FunctionUpdated=true;}
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
/*  TO Be debugged  000000000000000000if (!buttonpressed&&!LeftButtonState) {
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonLeft(MenuLevel);
    MenuLevel=MenuLevel-1; // nb cannot change MenuLevel in a function that called with menulevel as a variable,
           if (MenuLevel<=0){MenuLevel=3;}
    } 
  */  
#ifdef Rotary
if (millis()-ButtonPressTimer>=500) {buttonpressed=false;}// sets relatively slow repetition rate for push buttons
#endif
#ifndef Rotary
if (millis()-ButtonPressTimer>=200) {buttonpressed=false;}// sets faster repetition rate if no rotary sw present push buttons
#endif 


  ArduinoOTA.handle();

  // turn on the LED
 digitalWrite(LED_BUILTIN, LOW);
 // delay(1);
}
