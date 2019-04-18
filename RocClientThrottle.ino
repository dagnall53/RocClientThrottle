#define ver 36
#include "Secrets.h"
#define Rotary_Switch  // comment this out if not using the additional Rotary_Switch switch 
#ifdef Rotary_Switch
 #include <Rotary.h>   ///https://github.com/brianlow/Rotary
 Rotary r= Rotary(EncoderPinA, EncoderPinB);  //D9 D4
 long ThrottlePosition;
 long LastThrottlePosition;
 bool Encoder_Timeout;
 bool UpdatedRotaryMovementUsed;
 long Encoder_TimeoutAt;
 uint8_t Volts_Calibration;
#endif

// new stuff for buttons
#include <JC_Button.h>          // https://github.com/JChristensen/JC_Button
const unsigned long
    LONG_PRESS(1000);
    
bool     Fourway_SW_Menu_Action_Enabled;   // to assist with  the menu level change in four way switchto avoid immediately sending a function action 
bool     Menu_Action_Enabled;   // to assist with  the menu level change to avoid immediately sending a function action 
bool PowerON;                   // to assist with making the menu work with vry lng press for power on 
Button UpButtonButton(UpButton);       // define the buttons for the JC button functionality
Button DownButtonButton(DownButton);       // Button(pin, dbTime, puEnable, invert)
Button RightButtonButton(RightButton);       // Button(pin, 25ms default, enabled pullup default, default true for invert.
Button SelectButtonButton(SelectButton);       // 
Button LeftButtonButton(LeftButton); 


float analog_FP;
int batt_calibration; 

#include <SSD1306.h> // alias for `#include "SSD1306Wire.h"` //https://github.com/ThingPulse/esp8266-oled-ssd1306
// connect to display using pins D1, D2 for I2C on address 0x3c with esp32, 5,4 with esp32?
 SSD1306  display(0x3c, OLED_SDA, OLED_SCL);
// #include <PubSubClient.h> is included in MQTT.cpp.. Very important to read the notes there...

#include "Terrier.h"

#include <ArduinoOTA.h>
#ifdef _Use_Wifi_Manager
       #include <WiFiManager.h>
#else
       String wifiSSID = SSID_RR;
       String wifiPassword = PASS_RR; 
#endif
int disconnected;
       int BrokerAddr; 
String NameOfThisThrottle=ThrottleNameDefault;
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
uint8_t wifiaddr;
uint8_t ip0;
uint8_t ip1;
uint8_t    subIPH;
uint8_t    subIPL;
IPAddress ipBroad;
IPAddress mosquitto;
int connects;
bool  WIFI_SETUP;

uint32_t LoopTimer;
uint32_t DisplayTimer;
extern int hrs;
extern int mins;
extern int secs;
extern int clock_divider;
extern bool Clock_Freeze;
uint32_t LastSynchTime;
uint32_t LastSecTime;
uint32_t TIME;
 long StartupAt;
 bool LcpropsSent;

#include "MQTT.h" //new place for mqtt stuff (defines maxlocos
 //LOCO throttle settings from mqtt
 extern int LocoNumbers; //set in parse to actual number of locos in lc list
 extern String LOCO_id[MAXLOCOS+1];
 extern String SW_id[MAXLOCOS+1];
 extern int SW_bus[MAXLOCOS+1];
 extern int SW_addr[MAXLOCOS+1];

#include "Menu.h" //place for menu and display stuff, uses loco_id

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
int CurrentLocoIndex;
int fnindex;
int speedindex;
bool directionindex;
bool AllDataRead;

const int MaxAttribSize = 35;
char FoundAttrib[MaxAttribSize];// FoundAttrib= will be the attributes found by the attrib and attribcolon  functions;
char DebugMessage[128];
#include "NVSettingInterface.h"  // location for te EEPROM write and interface stuff
// allows call to check buttonstate

bool SelectPressed(){
   return !(digitalRead(SelectButton));
}



// These OLED display functions allow calling from anywhere, including my libraries.
void Picture(){
  display.clear(); 
  display.drawXbm(1,1, Terrier_Logo_width, Terrier_Logo_height, Terrier_Logo); 
 //  display.display();
}


void OLED_5_line_display_p(String L1,String L2,String L3,String L4,String L5){
   display.clear();
  Picture();
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 0, L1);
  display.drawString(64, 10, L2);
  display.drawString(64, 22, L3);
  display.drawString(64, 34, L4);
  display.drawString(64, 46, L5);
  display.display();
}
void OLED_5_line_display(String L1,String L2,String L3,String L4,String L5){
   display.clear();
 // Picture();
  display.setFont(ArialMT_Plain_10);
 display.drawString(64, 0, L1);
  display.drawString(64, 10, L2);
  display.drawString(64, 22, L3);
  display.drawString(64, 34, L4);
  display.drawString(64, 46, L5);
  display.display();
}

void ConnectionPrint() {
 char MsgTemp[127];
 int cx;
  Serial.println(F("-----------------------------------------------------------"));
  Serial.print (F("---------- Connected to SSID:"));
  Serial.print(WiFi.SSID());
  Serial.print(" / ");Serial.print(WiFi.psk());// added as part of an investigation into wifo connection
  Serial.print(F(" (EEPROMSet: SSID:"));// added as part of an investigation into wifo connection
  Serial.print(wifiSSID);              // added as part of an investigation into wifo connection
  Serial.print(F(" PW:"));              // added as part of an investigation into wifo connection
  Serial.print(wifiPassword);          // added as part of an investigation into wifo connection
  Serial.print(") ");                  // added as part of an investigation into wifo connection
  Serial.print(F("  IP:"));
  Serial.println(WiFi.localIP());
 }

void Banner(){
  String MSGText1;String MSGText2;
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  MSGText1="Looking for <";MSGText1+=wifiSSID;MSGText1+="> ";
  MSGText2="code <Ver:";MSGText2+=ver;MSGText2+="> ";
  OLED_5_line_display_p(""," ",MSGText1,MSGText2,"");
  Serial.println(F("-----------------------------------------------------------"));
  Serial.println(F("            This is Rocrail Client Throttle ")); 
  Serial.print  (F("            Named      \""));Serial.print(NameOfThisThrottle);Serial.println("\" ");
    Serial.print(F("-------------------- limit "));
  Serial.print(MAXLOCOS);
  Serial.println(F( " locos -----------------------"));
    Serial.print(F(  "                    revision:"));
  Serial.println(ver);
  Serial.println(F("-----------------------------------------------------------"));
  delay(100);
}


void _SetupOTA(String StuffToAdd){
  String Name;
  // ota stuff  Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]


 Name="RocMouse<";
 Name=Name+StuffToAdd;
 Name=Name+">";
 Serial.printf("------ Setting OTA Hostname <%s>\n",Name.c_str()); 
 Serial.println(F("-----------------------------------------------------------"));
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

void ConnectToWifi(String WiSSID, String Password, int Broker){
//Void Status(){
 String MSGText;String MSGText1;String MSGText2;
 int counter;
  delay(100);
  #ifdef _Use_Wifi_Manager
     WiFiManager wifiManager;  // this  stores ssid and password invisibly  !!
    //reset settings - for testing
    //wifiManager.resetSettings();
    wifiManager.autoConnect("ROCNODE ESP AP");  //ap name (with no password) to be used if last ssid password not found
  #else    

    WiFi.mode(WIFI_STA);  //Alternate "normal" connection to wifi
    #ifdef ESP32
      // nothing needed here, the ESP32 doe not have the 'set output power' function
    #else
      WiFi.setOutputPower(30);//  0 sets transmit power to 0dbm to lower power consumption, but reduces usable range.. try 30 for extra range
    #endif
   MSGText1="Trying SSID<";  MSGText1+=  WiSSID;  MSGText1+="> ";
   OLED_5_line_display_p("",""," ",MSGText1,"");
   MSGText1="- Trying to connect to SSID<";  MSGText1+=  WiSSID;  MSGText1+="> "; // longer message for serial interface...
   MSGText2+=" password <";  MSGText2+=  Password;  MSGText2+="> "; 
   counter=0;
   Serial.println(F("-----------------------------------------------------------"));
   Serial.print (MSGText1);Serial.print(MSGText2);Serial.print(" BrokerAddr:");Serial.println(Broker);
   WiFi.begin(WiSSID.c_str(), Password.c_str());
   delay(100);
   while (WiFi.status() != WL_CONNECTED) {delay(100);Serial.print("."); counter++; if (counter>=30){counter=0;Serial.println("");} }
 
 #endif // not using wifi manager

        //if you get here you should have connected to the WiFi
        //  Serial.println();Serial.println(F("--Connected--"));
 
   ipBroad = WiFi.localIP();
   subIPH = ipBroad[2];
   subIPL = ipBroad[3];
   wifiaddr = ipBroad[3];
   ConnectionPrint();  // display connection screen
  //   ++++++++++ MQTT setup stuff   +++++++++++++++++++++
   mosquitto[0] = ipBroad[0]; mosquitto[1] = ipBroad[1]; mosquitto[2] = ipBroad[2];
   mosquitto[3] = Broker; // mosquitto address 
   Serial.print(F("--------Mosquitto will first try to connect to:"));
   Serial.println(mosquitto);
   MQTT_ReConnect();  
   //if you get here you should have connected to the MQTT broker but MQTT_ReConnect() in MQTT includes code to search addr 3 to 75 if its not at the expected address
    Serial.println(F("----------------MQTT NOW setup ----------------")); 
    WIFI_SETUP=true;
  _SetupOTA(NameOfThisThrottle); // now we  have set the ota update with nickname ThrottleName 
                           //Note In Arduino Ports: You will also see the IP address so you can select which throttle of many with the same name to update OTA
  MSGText=" IP:";MSGText+=ipBroad[0];MSGText+=":";MSGText+=ipBroad[1];MSGText+=":";MSGText+=ipBroad[2];MSGText+=":";MSGText+=wifiaddr;
  OLED_5_line_display_p("",""," ","WiFi Connected",MSGText);delay(1000);
  }




void PrintLocoSettings(){
        for (int loco=1; loco<= LocoNumbers; loco++){
            Serial.print(F("Attributes stored for <"));Serial.print(loco);Serial.print(F("> <"));Serial.print(LOCO_id[loco]);
         
      Serial.println("");}
}


char MsgTextBattVolts[32];
void BatteryDisplay(int avg){
  int cx,BattDec,BattInt; 
  long Battery;

   Battery= ReadADC(avg); // update only at 1 second updates 
  BattInt=Battery/100;BattDec=Battery-(100*BattInt); 
  if (BattDec<=9){ cx=sprintf(MsgTextBattVolts,"%d.0%dV",BattInt,BattDec);}
                 else{cx=sprintf(MsgTextBattVolts,"%d.%dV",BattInt,BattDec);} 
}


void setup() {
  bool UsedDefaults;
  
  
  String msg; // local variable for any string message builds
  // init serial port 

  Serial.begin(115200);
  delay(500); // allow time for serial to set up
  Serial.println("-----Serial Port Running------"); 
  // set the builtin LED pin to work as an output
  pinMode(On_Board_LED, OUTPUT);
  pinMode (ADC_IN,INPUT);
  WIFI_SETUP=false;
  EEPROM.begin(EEPROM_Size); //Initialize EEPROM
  UsedDefaults=false;
  // TestFillEEPROM(72); // used for test only to check read eeprom etc functions
      wifiSSID = read_String(ssidEEPROMLocation);
      wifiPassword = read_String(passwordEEPROMLocation);
      BrokerAddr = EEPROM.read(BrokerEEPROMLocation);
      NameOfThisThrottle = read_String(ThrottleNameEEPROMLocation);
  if ( (SSID_RR != "Router Name") && (wifiSSID == "default") ){
      Serial.println("Using Secrets.h settings"); 
      wifiSSID = "default"; // just for the eeprom setting... so we can change if we want
      wifiPassword = PASS_RR;
      BrokerAddr = BrokerAddrDefault;
      NameOfThisThrottle = ThrottleNameDefault;
      WriteWiFiSettings();
      wifiSSID = SSID_RR; // but use the Secrets.h value 
      }
 

       
      Volts_Calibration = EEPROM.read(CalEEPROMLocation);
  
  if (Volts_Calibration == 0) {Volts_Calibration = 91;}
  Serial.print(" Broker addr:");Serial.println(BrokerAddr);
  Serial.print(" Battery Volts_Calibration Factor:");Serial.println(Volts_Calibration);
  if ((wifiSSID=="")||(wifiSSID.length()>=90)){wifiSSID=SSID_RR;UsedDefaults=true;Serial.println("Using Default SSID");}         // if empty, or if bigger than 90 use the secrets default
  if ((wifiPassword=="")||(wifiPassword.length()>=90)){wifiPassword=PASS_RR;UsedDefaults=true;Serial.println("Using Default Password");} // if empty, or if bigger than 90 use the secrets default
  if ((NameOfThisThrottle=="")||(NameOfThisThrottle.length()>=90)){NameOfThisThrottle=ThrottleNameDefault;UsedDefaults=true;Serial.println("Using Default Throttlename");} // if empty, or if bigger than 90 use the secrets default
  if ((BrokerAddr==0)||(BrokerAddr==255)){BrokerAddr=BrokerAddrDefault;UsedDefaults=true;Serial.println("Using Default Broker address");}   // zero and 255 are not  valid Ip for the broker, use default instead
  if (UsedDefaults){WriteWiFiSettings();}
  Banner(); 
  SelectButtonButton.begin(); // start the JC button functions
  CheckForSerialInput(); // do this before you set any other  pin definitions..

  UpButtonButton.begin(); // start the JC button functions
  DownButtonButton.begin(); // start the JC button functions
  RightButtonButton.begin(); // start the JC button functions
 
  LeftButtonButton.begin(); // start the JC button functions

  // init the display

  connects=0;
  ConnectToWifi(wifiSSID,wifiPassword,BrokerAddr);
  //Status();  // where we try to log in to WiFi

  #ifdef Rotary_Switch
      pinMode(EncoderPinA, INPUT_PULLUP);   // Rotary_Switch encoder pins are not using JC button library
      pinMode(EncoderPinB, INPUT_PULLUP); 
      //ThumbWheel.write(0);
      UpdatedRotaryMovementUsed=true;      
      ThrottlePosition  = 0;
      LastThrottlePosition=0;
  #endif 

      
  
  hrs=0;
  mins=0;
  secs=0;
  clock_divider=1;
  Clock_Freeze=false;
  buttonpressed=false;
  MenuLevel=0;
  switchindex=1;
  locoindex=0;
  CurrentLocoIndex=-1; // set to somethging we cannot have to allow the consistent triggering of the get loco data function in menu
  Menu_Action_Enabled=false;
  Fourway_SW_Menu_Action_Enabled=false;
  PowerON=false;
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
  LastSynchTime=StartupAt;
  LastSecTime=StartupAt;
  clock_divider=1;
  LcpropsSent=false; 

  GetLocoList();
  DisplayTimer=millis();
  BatteryDisplay(100);
  Serial.print("Battery Voltage is ");Serial.println(MsgTextBattVolts);

  
}

long ReadADC(int avg){
    long AnalogAvg;int j;long analog_value,BatteryVx100;
    #ifdef ESP32
    analogReadResolution(10);
    analogSetAttenuation(ADC_6db); //6db range is 1024 counts to 2v approx 
    #endif
    j=0; AnalogAvg=0;
    for (int i=0;i<=avg;i++){
      #ifdef ESP32
      analog_value =analogRead(ADC_IN);
      #endif
      #ifdef ESP8266
      analog_value =analogRead(ADC_IN);
      #endif
       if ((analog_value>=100)&&(analog_value<=1500)){
             j=j+1;AnalogAvg = AnalogAvg+analog_value;}
             }
    if (j>=1){analog_value=AnalogAvg/j;}else{
                        #ifdef ESP32
                              analog_value =analogRead(ADC_IN);
                        #endif
                        #ifdef ESP8266
                              analog_value =analogRead(ADC_IN);
                        #endif
                        } // average or just last chance at reading!
    BatteryVx100=analog_value;// 
    BatteryVx100=BatteryVx100*Volts_Calibration; //~x100
    BatteryVx100=BatteryVx100/Cal_factor; 
   return BatteryVx100;
}

 void MQTT_DO(void){
    // MQTT stuff, & check MQTT is connected.. WiFi conection test is separate!
  
  if (!MQTT_Connected()) {MQTT_ReConnect(); delay(100);}           // was if (!client.connected()) 
      MQTT_Loop(); // for client.loop(); //gets wifi messages etc..
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
   for (int i=0; i<=(N_Functions) ;i++) {
    String msg;
    msg="F";msg=msg+i;
    FunctionName[i]=msg;
    FunctionState[i]=false;
    FunctionStateKnown[i]=false; 
    FunctionTimer[i]=0;
    } 
  MQTTSend("rocrail/service/client",MsgTemp);

}



void Do_RotarySW(){
  #ifdef Rotary_Switch
    long ThrottlePos;
    unsigned char result=r.process();
     if (result == DIR_NONE) {
    // do nothing
                             }
    else if (result == DIR_CW) {
       if(!Encoder_Timeout){ Encoder_Timeout=true; Encoder_TimeoutAt=millis();ThrottlePos=ThrottlePos+1;
       Serial.print("<"); UpdatedRotaryMovementUsed=false;ButtonUp(MenuLevel);LastThrottlePosition=ThrottlePosition;
                               }
                                }
  else if (result == DIR_CCW) {
       if(!Encoder_Timeout){ Encoder_Timeout=true; Encoder_TimeoutAt=millis();ThrottlePos=ThrottlePos-1;
       Serial.print(">"); UpdatedRotaryMovementUsed=false;ButtonDown(MenuLevel);LastThrottlePosition=ThrottlePosition;
       
                           }
  }
  #endif
}

void Fourwayswitch(){
if (UpButtonButton.wasPressed()){
     if (Menu_Action_Enabled){ Menu_Action_Enabled=false;
       Serial.print("U"); ButtonUp(MenuLevel);
                               }}
      
if (DownButtonButton.wasPressed()){
    if (Menu_Action_Enabled){Menu_Action_Enabled=false;
       Serial.print("D"); ButtonDown(MenuLevel);
    } }
    
if ((MenuLevel==0)&& RightButtonButton.wasPressed()) {// change from loco select to speed menu
        if (Fourway_SW_Menu_Action_Enabled){if (CurrentLocoIndex != locoindex){GetLocoFunctions(locoindex); CurrentLocoIndex=locoindex ;} MenuLevel=1;Fourway_SW_Menu_Action_Enabled=false;}} 

if ((MenuLevel==1) && RightButtonButton.wasPressed()) { // change from speed to fn menu
        if (Fourway_SW_Menu_Action_Enabled){MenuLevel=2; Fourway_SW_Menu_Action_Enabled=false;}} // disable function send for the First "button released" that will be seen on entering Function setting
    
if ((MenuLevel==2) && RightButtonButton.wasPressed()) { // change back to select loco menu
        if (Fourway_SW_Menu_Action_Enabled){MenuLevel=0; Fourway_SW_Menu_Action_Enabled=false;}} 
        
if (RightButtonButton.wasReleased()){Fourway_SW_Menu_Action_Enabled=true;} // resets action enable if button not pressed at all -- Essential on startup to ensure a single press can trigger the action to go to menulevel 1        
   
}


void ClockUpdate(){
 if(!Clock_Freeze){  // Rocrail TIME is time since epoch start Seconds since Jan 01 1970. (UTC)
  if (((LoopTimer-LastSynchTime)*clock_divider) >=1000){secs=secs+1;LastSynchTime=LoopTimer;}
  
  if (secs >= 59.5){secs=0; mins=mins+1;LastSynchTime=LoopTimer;  // Serial.print("%"); // do stuff here for tests every second..for debug
          if (mins>=60){hrs=hrs+1;mins=0;}
             if (hrs>=25){hrs=1;}} // 
  }else{                } //
 
}  


boolean WiFiReturns() {  //https://github.com/esp8266/Arduino/issues/4161#issuecomment-378086456
if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) return 0;
  switch (WiFi.status()) {
    case WL_NO_SHIELD: return 0;
    case WL_IDLE_STATUS: return 0;
    case WL_NO_SSID_AVAIL: return 0;
    case WL_SCAN_COMPLETED: return 1;
    case WL_CONNECTED: return 1;
    case WL_CONNECT_FAILED: return 0;
    case WL_CONNECTION_LOST: return 0;
    case WL_DISCONNECTED: return 0;
    default: return 0;
  }
}

void loop() {
    // turn on the LED
  digitalWrite(On_Board_LED, LOW);
   if (!WiFiReturns()) { Serial.println("Lost Connection Trying Reconnect");
    WiFi.reconnect(); delay(200);digitalWrite(On_Board_LED, HIGH);delay(200); //Connection OFF - conexión OFF
  } else { //conexión ON - Connection ON
    
  
   //while (WiFi.status() != WL_CONNECTED) {Serial.println("Lost Connection Trying Reconnect");wifi_connect();delay(500); }
  LoopTimer = millis(); // idea is to use LoopTimer instead of millis to ensure synchronous behaviour in loop
  ClockUpdate();
   
  //recvWithEndMarker();  // why???
  showNewData();

  UpButtonButton.read(); // start the JC button functions
  DownButtonButton.read(); // start the JC button functions
  RightButtonButton.read(); // start the JC button functions
  SelectButtonButton.read(); // start the JC button functions
  LeftButtonButton.read(); // start the JC button functions

  #ifdef Rotary_Switch
   Do_RotarySW();
    if ((millis()-Encoder_TimeoutAt)>=50){Encoder_Timeout=false;} // sets max click repetition rate for encoder
  #endif 
 
  display.clear();  
  MQTT_DO();   
  
  DoDisplay(MenuLevel);


  // turn off the LED
  digitalWrite(On_Board_LED, HIGH);
  delay(1);// essentially this is the main loop delay

// completely new Button and rotary switch menu


  Fourwayswitch(); // up and down same as rotary rotates...

//Jans Menu idea with just rotary switch


  if ((MenuLevel==0) && SelectButtonButton.pressedFor(LONG_PRESS)) {
        if (Menu_Action_Enabled){
        MQTTSend("rocrail/service/client","<sys cmd=\"stop\"/>");
        PowerON=false;
        drawRect();
        Menu_Action_Enabled=false;}  }

  if ((MenuLevel==0) && SelectButtonButton.pressedFor((LONG_PRESS*2))) {// VERY LONG Press
        if (!PowerON){ //Serial.println("Very long press"); ///use PowerON bool to make this send once only
        MQTTSend("rocrail/service/client","<sys cmd=\"go\"/>");
        fillRect();
        PowerON=true;}  
        }
    
  if ((MenuLevel==0)&& SelectButtonButton.wasReleased() && !SelectButtonButton.pressedFor(LONG_PRESS)) {// change from loco select to speed menu
        if (Menu_Action_Enabled){if (CurrentLocoIndex != locoindex){GetLocoFunctions(locoindex); CurrentLocoIndex=locoindex ;} MenuLevel=1;Menu_Action_Enabled=false;}} 

  if ((MenuLevel==1) && SelectButtonButton.pressedFor(LONG_PRESS)) { // zero speed on long press 
        if (Menu_Action_Enabled){speedindex=0;SetLoco(locoindex,speedindex);Menu_Action_Enabled=false;}} // speed = 0 

  if ((MenuLevel==1) && SelectButtonButton.wasReleased()&& !SelectButtonButton.pressedFor(LONG_PRESS)) { // change from speed to fn menu
        if (Menu_Action_Enabled){MenuLevel=2; Menu_Action_Enabled=false;}} // disable function send for the First "button released" that will be seen on entering Function setting
    
  if ((MenuLevel==2) && SelectButtonButton.pressedFor(LONG_PRESS)) { //Do fn action on long press
        if (Menu_Action_Enabled){Do_Function(locoindex,fnindex);Menu_Action_Enabled=false;}}

  if ((MenuLevel==2) && SelectButtonButton.wasReleased() && !SelectButtonButton.pressedFor(LONG_PRESS)) { // change back to select loco menu
        if (Menu_Action_Enabled){MenuLevel=0; Menu_Action_Enabled=false;}} 

  if (SelectButtonButton.wasReleased()){Menu_Action_Enabled=true;} // after any level change, wait for button release before allowing other actions
  if (SelectButtonButton.releasedFor(LONG_PRESS)){Menu_Action_Enabled=true;} // resets action enable if button not pressed at all -- Essential on startup to ensure a single press can trigger the action to go to menulevel 1        
           
  
  ArduinoOTA.handle();

  // turn off the LED
  digitalWrite(On_Board_LED, HIGH);

  }// only do if connected to wifi
}


