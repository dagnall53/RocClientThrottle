#define ver 003

//#define Rotary  // comment this out if not using the additional rotary switch or if you want speed increments of 10 and not the locolist Vx.. steps

#include <SSD1306Wire.h>  //https://github.com/ThingPulse/esp8266-oled-ssd1306


#include "Terrier.h"
#include "MQTT.h"; //new place for mqtt stuff
#include <ArduinoOTA.h>
#ifdef _Use_Wifi_Manager
       #include <WiFiManager.h>
#else
       #include "Secrets.h";
       String wifiSSID = SSID_RR;
       String wifiPassword = PASS_RR; 
#endif
#include <ESP8266WiFi.h>
uint8_t wifiaddr;
uint8_t ip0;
uint8_t ip1;
uint8_t    subIPH;
uint8_t    subIPL;
//uint16_t HandControlID;

/* Encoder by Pul Stoffregen 
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 */


#ifdef Rotary
 #include <Encoder.h>
 const int EncoderPinA = D9;     // the number of the A pushbutton pin (common is connected to ground
 const int EncoderPinB = D4;     // the number of the B pushbutton pin (common is connected to ground
 Encoder ThumbWheel(EncoderPinA, EncoderPinB);  //D3 D4
 long ThrottlePosition;
 long LastThrottlePosition;
 long ThrottleClicks;
 bool EncoderMoved;
 bool FunctionUpdated;
 long EncoderMovedAt;
#endif


//LOCO throttle settings from mqtt
extern int LocoNumbers; //set in parse to actual number of locos in lc list

extern String LOCO_id[MAXLOCOS];
extern String LOCO_V_min[MAXLOCOS];
extern String LOCO_V_mid[MAXLOCOS];
extern String LOCO_V_cru[MAXLOCOS];
extern String LOCO_V_max[MAXLOCOS];
extern String LOCO_spcnt[MAXLOCOS];
extern int SW_bus[MAXLOCOS];
extern int SW_addr[MAXLOCOS];

//tern String LOCO_V_cru[MAXLOCOS];
//extern String LOCO_V_max[MAXLOCOS];
//extern String LOCO_spcnt[MAXLOCOS];

IPAddress ipBroad;
IPAddress mosquitto;
int connects;

// connect to display using pins D1, D2 for I2C on address 0x3c
SSD1306Wire  display(0x3c, D1, D2);
 #include "Menu.h"; //place for menu and display stuff

 
int y;
const int buttonPin3 = D3;     // the number of the pushbutton pin
const int buttonPin5 = D5;     // the number of the pushbutton pin
const int buttonPin6 = D6;     // the number of the pushbutton pin
const int buttonPin7 = D7;     // the number of the pushbutton pin
bool buttonState3 = 0;         // variable for reading the pushbutton status
bool buttonState4 = 0;         // variable for reading the pushbutton status
bool buttonState5 = 0;         // variable for reading the pushbutton status
bool buttonState6 = 0;         // variable for reading the pushbutton status
bool buttonState7 = 0;         // variable for reading the pushbutton status
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
void OLED_Display(char* L1,char* L2,char* L3){
  display.clear();
 // Picture();
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

  WiFi.setOutputPower(20); //  0 sets transmit power to 0dbm to lower power consumption, but reduces usable range.. try 30 for extra range

#ifdef _Use_Wifi_Manager
   WiFiManager wifiManager;  // this  stores ssid and password invisibly  !!
  //reset settings - for testing
  //wifiManager.resetSettings();
  wifiManager.autoConnect("ROCNODE ESP AP");  //ap name (with no password) to be used if last ssid password not found
#else    

  WiFi.mode(WIFI_STA);  //Alternate "normal" connection to wifi
  WiFi.setOutputPower(30);
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
            Serial.print(F("> Vmin:"));Serial.print (LOCO_V_min[loco]);
            Serial.print(F(" Vmid:"));Serial.print (LOCO_V_mid[loco]);
            Serial.print(F(" Vcru:"));Serial.print (LOCO_V_cru[loco]);
            Serial.print(F(" Vmax:"));Serial.print (LOCO_V_max[loco]);
            Serial.print(F(" spdcnt:"));Serial.print (LOCO_spcnt[loco]);
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


 Name="RC<";
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
  Serial.begin(115200);
 
  // set the builtin LED pin to work as an output
 pinMode(LED_BUILTIN, OUTPUT);
  
 pinMode(buttonPin3, INPUT_PULLUP);
 pinMode(buttonPin5, INPUT_PULLUP);
 pinMode(buttonPin6, INPUT_PULLUP);
 pinMode(buttonPin7, INPUT_PULLUP);
 
#ifdef Rotary
pinMode(EncoderPinA, INPUT_PULLUP);   // rotary encoder pins
pinMode(EncoderPinB, INPUT_PULLUP);
#endif
 
  // init the display
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
 
  Picture();
  display.drawString(64, 32, "Looking for WiFi");display.display();
  delay(1000);

 connects=0;
 Status();
 _SetupOTA("SwitchTool"); // now we  have set the nickname 
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
RightIndexPos=3;
LeftIndexPos=0;
directionindex=true;
LocoNumbers=0; 
AllDataRead=false;
LOCO_id[0]="this is an unikely name";
}

 void MQTT_DO(void){
    // MQTT stuff, & check we are connected.. 
  if (!MQTT_Connected()) {reconnect();}           // was if (!client.connected()) {
      
  MQTT_Loop(); // for client.loop(); //gets wifi messages etc..
 }
 
void loop() {
buttonState3 = digitalRead(buttonPin3);
buttonState5 = digitalRead(buttonPin5);
buttonState6 = digitalRead(buttonPin6);
buttonState7 = digitalRead(buttonPin7);
//Serial.print(" Buttons 3<");Serial.print(buttonState3);Serial.print("  5<");Serial.print(buttonState5);Serial.print(">  6<");Serial.print(buttonState6);Serial.print(">  7<");Serial.print(buttonState7);Serial.println(">");
 
#ifdef Rotary
  long ThrottlePos;
  ThrottlePos = ThumbWheel.read();
  if (ThrottlePos != ThrottleClicks ) {
    ThrottleClicks = ThrottlePos;
    ThumbWheel.write(ThrottleClicks);
    ThrottlePosition=int(ThrottleClicks/4);
   //Serial.print("Throttleclicks  = ");Serial.print(ThrottleClicks); // un comment for debug
   //Serial.print (" throttlepos:"); Serial.print(ThrottlePosition);   // un comment for debug
   //Serial.print (" lastthrottlepos:"); Serial.println(LastThrottlePosition);   // un comment for debug
   
if(!EncoderMoved){    EncoderMoved=true; EncoderMovedAt=millis();
     if ((ThrottlePosition-LastThrottlePosition)>=1){Serial.print("<");
          FunctionUpdated=false;ButtonUp(MenuLevel);LastThrottlePosition=ThrottlePosition;}
     if ((ThrottlePosition-LastThrottlePosition)<=-1){Serial.print(">");
          FunctionUpdated=false;ButtonDown(MenuLevel);LastThrottlePosition=ThrottlePosition;}
   // if (abs(ThrottlePosition-LastThrottlePosition)>=1){FunctionUpdated=false;LastThrottlePosition=ThrottlePosition;}
}
             }
 //Serial.print("AFTER Rotaty Buttons 3<");Serial.print(buttonState3);Serial.print("  5<");Serial.print(buttonState5);Serial.print(">  6<");Serial.print(buttonState6);Serial.print(">  7<");Serial.print(buttonState7);Serial.println(">");

if ((millis()-EncoderMovedAt)>=50){EncoderMoved=false;} // sets repetition rate for encoder
 //   if (!FunctionUpdated){SetLoco(locoindex,ThrottlePosition); FunctionUpdated=true;}
#endif 
 
  display.clear();   // clear the screen get loco list if not  
  
  MQTT_DO();
  DoDisplay(MenuLevel);


  // turn off the LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1);// essentially this is the main loop delay


//Serial.print(" Buttons 3<");Serial.print(buttonState3);Serial.print("  4<");Serial.print(buttonState4);Serial.print("  5<");Serial.print(buttonState5);Serial.print(">  6<");Serial.print(buttonState6);Serial.print(">  7<");Serial.print(buttonState7);Serial.println(">");
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (!buttonpressed&&!buttonState3) {
    buttonpressed=true;ButtonPressTimer=millis();
   ;ButtonRight(MenuLevel);
          MenuLevel=MenuLevel+1; // nb cannot change MenuLevel in a function that called with menulevel as a variable,
           if (MenuLevel>= 3){MenuLevel=0;}
      }
  if (!buttonpressed&&!buttonState5) {
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonSelect(MenuLevel);     
      } 
  if (!buttonpressed&&!buttonState6) {
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonUp(MenuLevel);
    } 
  if (!buttonpressed&&!buttonState7) {
    buttonpressed=true;ButtonPressTimer=millis();
    ButtonDown(MenuLevel);
    } 
#ifdef Rotary
if (millis()-ButtonPressTimer>=600) {buttonpressed=false;}// sets repetition rate for push buttons
#endif
#ifndef Rotary
if (millis()-ButtonPressTimer>=200) {buttonpressed=false;}// sets faster repetition rate if no rotary sw present push buttons
#endif 
  ArduinoOTA.handle();

  // turn on the LED
  //digitalWrite(LED_BUILTIN, LOW);
 // delay(1);
}
