
#include "MQTT.h";
#include <Arduino.h> //needed 
//  #include "Directives.h";
#define SignalON LOW  // defined so I can change the "phase of the SignalLED" easily.
#define SignalOFF HIGH


// ----------Put these in pubsubclient.h
#define MQTT_VERSION MQTT_VERSION_3_1 //Rocrail needs to use V 3_1 not 3_1_1 (??)
#define MQTT_MAX_PACKET_SIZE 10000   // lclist is LONG...!
// ---------- This is vital!!

#include <PubSubClient.h>
#include <WiFiClient.h>
WiFiClient espClient;

PubSubClient client(espClient);

extern IPAddress mosquitto;

//#include "Globals.h";

byte payload[12000];
extern uint8_t ip0;
extern uint8_t ip1;
extern uint8_t subIPH;
extern uint8_t subIPL;
extern int connects;

char* Nickname;
uint8_t hrs;
uint8_t mins;
uint8_t secs;  
char DebugMsg[127];
uint8_t NodeMCUPinD[12];

boolean  Data_Updated;

uint32_t EPROM_Write_Delay;
uint32_t Ten_Sec;

uint32_t LcPropsEnabled;
boolean Message_Decoded;
extern bool AllDataRead;

byte ParseIndex;
int LocoNumbers;
String LOCO_id[MAXLOCOS];
String LOCO_V_min[MAXLOCOS];
String LOCO_V_mid[MAXLOCOS];
String LOCO_V_cru[MAXLOCOS];
String LOCO_V_max[MAXLOCOS];
String LOCO_spcnt[MAXLOCOS];
String ThisID;
extern void Picture();

int Count(unsigned int start,char* id, byte* data,unsigned int Length) {// to find string id in data and count  (max 65k)
 int count;int allsame;
 unsigned int AttribLength;
 bool same;
 count=0;
 AttribLength = strlen(id)-1;
int Y;
 for (unsigned int x=start ; x<= Length-AttribLength-1; x++){
 //  for (unsigned int x=0 ; x<= 160; x++){
  allsame=0;
  for (int y=0;y<=(AttribLength);y++){Y=y;
    if (id[y]==char(data[x+y])){allsame=allsame+1;}
        }
  if (allsame==AttribLength+1){count=count+1;
  }
 }
 return count;
}



char* Attrib(unsigned int Nth,char* id, byte* data,unsigned int Length) {// to find string id in data and count  (max 65k)
 int count;int allsame;int Y;
 unsigned int AttribLength;
 const int MaxAttribSize =30;
 char* FoundAttrib;
 count=0;
 //id.concat("\"");
 AttribLength = strlen(id)-1;

// Serial.println();Serial.print("starting search for entry <");Serial.print(Nth);Serial.println("> for <");Serial.print(id);Serial.print("> ... ");

  for (unsigned int x=0 ; x<= Length-AttribLength-20; x++){
  // for (unsigned int x=0 ; x<= 160; x++){
  allsame=0;
  
  FoundAttrib="";// instantiate string or get problems later!
  for (int fill=0; fill<=MaxAttribSize;fill++){FoundAttrib[fill]=0;  } // fill with null
  
  for (int y=0;y<=(AttribLength);y++){  Y=y; if (id[y]==char(data[x+y])){allsame=allsame+1; } 
      }//y loop
        if (allsame==AttribLength+1){count=count+1;} // count 
            if (count==Nth){
        // Serial.print("found Nth text at char<");Serial.print(x);Serial.print(">  is<");
               for (int z=0 ;z<=MaxAttribSize;z++) { 
                   //FoundAttrib[z]=char(data[x+Y+z+1]);
                   if (data[x+Y+z+1]==34){FoundAttrib[z]=0;
                   for (int fill=z; fill<=MaxAttribSize;fill++){FoundAttrib[fill]=0;  } // not needed with filled string with '0' ?
                   // Serial.println(">");
                   return FoundAttrib;}
                else{FoundAttrib[z]=char(data[x+Y+z+1]);
                  //Serial.print(char(data[x+Y+z+1]));  
                  }
         }// z loop
    }
    
    //if same
 
   } //x loop
 return FoundAttrib;
}




void ParsePropsLocoList(byte loco, byte* payload, unsigned int Length){
  // propslist gives just one loco at a time so the attrib nth is 1 every time. but will be stored in index loco
      LOCO_id[loco]=Attrib(1,"<lc id=\"",payload,Length);
      LOCO_V_min[loco]=Attrib(1," V_min=\"",payload,Length);
      LOCO_V_mid[loco]=Attrib(1," V_mid=\"",payload,Length);
      LOCO_V_cru[loco]=Attrib(1," V_cru=\"",payload,Length);
      LOCO_V_max[loco]=Attrib(1," V_max=\"",payload,Length);
      LOCO_spcnt[loco]=Attrib(1," spcnt=\"",payload,Length);
     LocoNumbers=loco+1;
     if (LocoNumbers>=MAXLOCOS-1) {LocoNumbers=MAXLOCOS-1;}
     Serial.print("Attributes for <");Serial.print(loco);Serial.print("> <");Serial.print(LOCO_id[loco]);
     Serial.print(">  V_min:");Serial.print (LOCO_V_min[loco]);
     Serial.print(" V_mid:");Serial.print (LOCO_V_mid[loco]);
     Serial.print(" V_cru:");Serial.print (LOCO_V_cru[loco]);
     Serial.print(" V_max:");Serial.print (LOCO_V_max[loco]);
     Serial.print(" spdcnt:");Serial.print (LOCO_spcnt[loco]);
     Serial.println(" ");
   }


extern int locoindex;
void MQTTFetch (char* topic, byte* payload, unsigned int Length) { //replaces rocfetch
  // do a check on length matching payload[7] ??
char PayloadAscii[100];

 Message_Decoded = false;
//  Do Clock synch   
  if ((strncmp("rocnet/ck", topic, 9) == 0)) { //==0 if n bytes are the same
     hrs = payload[12];
     mins = payload[13];
     secs = payload[14]; 
     Message_Decoded = true;
     PrintTime("--Clock Synch--");Serial.println(".");
   // return;
                                             }
  /// is it service info?
    if ((strncmp("rocrail/service/info", topic, 20) == 0)) {
    // Print message size whilst finding out if the pubsub client max message size is adequate..
//    Serial.println(F("----------Service Info Message----  Length: "));Serial.print(Length);Serial.print(F("  ="));
//    unsigned int y;
//    y= MQTT_MAX_PACKET_SIZE;
//    Serial.print((100*Length)/MQTT_MAX_PACKET_SIZE);Serial.println(F("% msg size used"));
    // Print message size whilst finding out if the pubsub client max message size is adequate..

// make PayloadAscii so we can use strncmp 
   for (int i=0; i<=50;i++){PayloadAscii[i]=char(payload[i]); }
       
   if (strncmp("<lc id=\"",PayloadAscii,8)==0){
      if ((ParseIndex<=MAXLOCOS)&& (!AllDataRead)){ 
      ThisID=Attrib(1,"<lc id=\"",payload,Length);
      if (ThisID==LOCO_id[locoindex]){//Serial.print("this is a response for somethng we are working with at the moment");
                                     }
         else{if (ThisID==LOCO_id[0]){//Serial.print(" Read this loco before "); //This is only triggered if the lcprops list is somehow re-triggered
                                        AllDataRead=true;}
                else{if (millis()<=LcPropsEnabled){LcPropsEnabled=millis()+1500;// give yourself another second and a half to get more data...
                                                   Serial.print(F(" Found loco props list  <"));Serial.print(ParseIndex);Serial.println(">");
                                                   ParsePropsLocoList(ParseIndex,payload,Length);  
                                                   ParseIndex++;}}
             }  
       Message_Decoded = true;}
      
      }  

     // return;     
    }
 // end of service info 
  
// if not decoded, then it  must be something else so print for debugging....

//debug printing
// known but unimportant (to me) messages
     if (strncmp("<exception text=",PayloadAscii,16)==0){
         Message_Decoded = true; }  
     if (strncmp("<clock divider=",PayloadAscii,15)==0){
         Message_Decoded = true; }
     if (strncmp("<program modid=",PayloadAscii,15)==0){
         Message_Decoded = true; }
     if (strncmp("<sw id=",PayloadAscii,7)==0){
         Message_Decoded = true; }
         //switch off debug
         Message_Decoded=true;
if (!Message_Decoded){
 bool NewLine;
 
//    Serial.println(F("----------Service Info Message----  Length: "));Serial.print(Length);Serial.print(F("  ="));
//    unsigned int y;
//    y= MQTT_MAX_PACKET_SIZE;
//    Serial.print((100*Length)/MQTT_MAX_PACKET_SIZE);Serial.println(F("% msg size used"));
 Serial.println(F("**  MQTT FETCHED - Not decoded"));
Serial.println("{");
 NewLine=false;
 for (long i = 0; i <= Length; i++) {
  // sometimes need to see exactly where the CR (\R OR 13) or  LF (\N OR 10) ARE
  if (payload[i]==13){Serial.print("{r}");}
  if (payload[i]==10){Serial.print("{n}");}
  Serial.write(char(payload[i]));
 if ((i%60)==0){NewLine=true;} 
 if(NewLine && payload[i]==32){Serial.println();NewLine=false;}
 }
  Serial.println("}");
 Serial.println("---------end of payload------------");
Message_Decoded = true;
}
  // end of debug printing...
 
} 
                                             



void MQTTSend (char* topic, char* payload) { //replaces rocsend
  uint8_t Length;
  digitalWrite (LED_BUILTIN, HIGH) ;  /// turn On
  Length = sizeof(payload);
  client.publish(topic, payload, Length);

}


//   ++++++++++ MQTT setup stuff   +++++++++++++++++++++
void  MQTT_Setup(void){
  
  client.setServer(mosquitto, 1883);   // Hard set port at 1833
  //client.setServer(mqtt_server, 1883); // old hard set version...
  client.setCallback(MQTTFetch);
}


boolean MQTT_Connected(void){
  return client.connected();
}

void MQTT_Loop(void){
    client.loop(); //gets wifi messages etc..
}
extern uint16_t HandControlID;

void DebugMsgSend (char* topic, char* Debug_Payload) { // use with mosquitto_sub -h 127.0.0.1 -i "CMD_Prompt" -t debug -q 0
  char DebugMsgLocal[127];
    char DebugMsgTemp[127];
  int cx;


  cx= sprintf ( DebugMsgTemp, " Controller:%d  Msg:%s", HandControlID,  Debug_Payload);

   // add timestamp to outgoing message
if ((hrs==0)&&(mins==0)){// not Synchronised yet..
  cx=sprintf(DebugMsgLocal," Time not synchronised yet %s",DebugMsgTemp);
   }
   else {cx=sprintf(DebugMsgLocal,"<%02d:%02d:%02ds> %s",hrs,mins,secs,DebugMsgTemp);
         }
//
    //Serial.printf("\n *Debug Message:%s Msg Length:%d \n",DebugMsgLocal,cx);
    Serial.printf("\n *Debug Message:%s  \n",DebugMsgLocal);

 
    if ((cx <= 120)) {
      client.publish(topic, DebugMsgLocal, strlen(DebugMsgLocal));
                     }
    if ((cx >= 120) && (strlen(Debug_Payload) <= 100)) {
      cx= sprintf ( DebugMsgLocal, "MSG-%s-", Debug_Payload);
      client.publish(topic, DebugMsgLocal, strlen(DebugMsgLocal));
                                          }// print just msg  line
    if (strlen(Debug_Payload) >= 101) {
      cx= sprintf ( DebugMsgLocal, "Controller:%d  Time %d:%d:%ds Msg TOO Big to print", HandControlID,  hrs, mins, secs);
      client.publish(topic, DebugMsgLocal, strlen(DebugMsgLocal));
    }

 }
void PrintTime(String MSG) {
/*      Serial.println("");
      Serial.print("@");
      Serial.print(hrs);
      Serial.print(":");
      if (mins <= 9){
        Serial.print("0");
      }
      Serial.print(mins);
      Serial.print(":"); 
      if (secs <= 9){
        Serial.print("0");
      }
      Serial.print(secs);
      Serial.print(" ");
      Serial.print(MSG);
      */
Serial.printf("@<%02d:%02d:%02ds>:%s",hrs,mins,secs,MSG.c_str());

}

  void DebugSprintfMsgSend(int CX){ // allows use of Sprintf function in the "cx" location
  DebugMsgSend ("debug", DebugMsg);
  delay(5);
}
extern uint8_t wifiaddr;
extern void OLED_Display(char* L1,char* L2,char* L3);
void reconnect() {    
  char DebugMsgTemp[127];
    int cx;

  char ClientName[80];
  char myName[15] = "HandControl:";
  sprintf(ClientName, "%s%i", myName, wifiaddr);
  // Loop until we're reconnected 
  //digitalWrite (LED_BUILTIN , SignalON) ; ///   turn on
  PrintTime(" Attempting MQTT (re)connection. Attempt #");
  Serial.println(connects);
  while (!client.connected()) {
    Serial.print("I am  <");Serial.print(ClientName);  Serial.print(">  ");
    Serial.print(" and will try to connect to MQTT broker at:");
    Serial.print(mosquitto);
  // Attempt to connect

    if (client.connect(ClientName)) {
      Serial.println();
      //DebugSprintfMsgSend( sprintf ( DebugMsg, "%s Connected at:%d.%d.%d.%d",ClientName,ip0,ip1,subIPH,subIPL));
   /* // this code only used if we are going to try a search for the broker..
      if (mosquitto[3] != RN[14] ){   //RN[14] is the MQQT broker address, save if changed
                                  RN[14]=mosquitto[3];
                                  WriteEEPROM();
                                  Data_Updated=true; 
                                  EPROM_Write_Delay = millis()+Ten_Sec; 
                                  }
     */ // we can advise this node is now connected 
     
      DebugSprintfMsgSend( sprintf ( DebugMsg, "%s Connected at:%d.%d.%d.%d",ClientName,ip0,ip1,subIPH,subIPL));
      Serial.println(DebugMsg);
        // ... and now subscribe to topics  http://wiki.rocrail.net/doku.php?id=rocnet:rocnet-prot-en#groups
      client.subscribe("rocrail/service/info", 1 ); // server information
      client.subscribe("rocnet/ck", 1 ); // Clock synch  information
     // client.subscribe("rocrail/service/client", 1 ); // an we see client data? test..       
     // delay(100);
     EPROM_Write_Delay = millis();
                                   } else {
     Serial.println(); Serial.print("Failed to connect to MQTT broker ");
     // this code only used if we are going to try a search for the broker..
     //connects=connects+1;
     // if (connects>=5){  mosquitto[3] = mosquitto[3]+1;
     // if (mosquitto[3]>=50){mosquitto[3]=3;}   }   // limit mqtt broker to 3-50 to save scan time
     delay(100);
     cx= sprintf ( DebugMsgTemp,  "at:%d.%d.%d.%d",mosquitto[0],mosquitto[1],mosquitto[2],mosquitto[3]);
     Serial.println (DebugMsgTemp); 
     OLED_Display("","looking for broker",DebugMsgTemp);
     Serial.println(" trying again ..."); 
     client.setServer(mosquitto, 1883);   // Hard set port at 1833  
      //FlashMessage(".... Failed connect to MQTT.. attempting reconnect",4,250,250);
      // Wait   before retrying  // perhaps add flashing here so when it stops we are connected?
      delay(100);
      //digitalWrite (LED_BUILTIN , SignalOFF) ; ///   turn OFF
                                         }
  }
}

