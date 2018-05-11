
#include "MQTT.h";
#include <Arduino.h> //needed 
//  #include "Directives.h";
#define SignalON LOW  // defined so I can change the "phase of the SignalLED" easily.
#define SignalOFF HIGH


// put these in pubsubclient.h
#define MQTT_VERSION MQTT_VERSION_3_1 //Rocrail needs to use V 3_1 not 3_1_1 (??)
#define MQTT_MAX_PACKET_SIZE 25000   // lclist is LONG...!

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


boolean Message_Decoded;
extern bool AllDataRead;
byte ParseIndex;
int SwitchNumbers;
int LocoNumbers;
String LOCO_id[MAXLOCOS];
String LOCO_V_min[MAXLOCOS];
String LOCO_V_mid[MAXLOCOS];
String LOCO_V_cru[MAXLOCOS];
String LOCO_V_max[MAXLOCOS];
String LOCO_spcnt[MAXLOCOS];
int SW_bus[MAXLOCOS];
int SW_addr[MAXLOCOS];
extern int switchindex;
extern int RightIndexPos;
extern int LeftIndexPos;
extern byte SW_all[9];

int Count(unsigned int start,char* id, byte* data,unsigned int Length) {// to find string id in data and count  (max 65k)
 int count;int allsame;
 unsigned int AttribLength;
 bool same;
 count=0;
 AttribLength = strlen(id)-1;
// Serial.println();Serial.print("length of string \"");Serial.print(id);// note  \"
// Serial.print("\" is:");Serial.println(AttribLength);
//for (int y=0;y<=(AttribLength);y++){
//  Serial.print("id --<");Serial.print(y);Serial.print("> ={");Serial.print(id[y]);Serial.println("}");
// }
// Serial.print("char(data) --<");
//for (int y=0;y<=50;y++){
//  Serial.print(char(data[y]));
//}Serial.println();

//Serial.print("starting count of <");Serial.print(id);Serial.println(">'s");
int Y;
 for (unsigned int x=start ; x<= Length-AttribLength-1; x++){
 //  for (unsigned int x=0 ; x<= 160; x++){
  allsame=0;
  for (int y=0;y<=(AttribLength);y++){Y=y;
    if (id[y]==char(data[x+y])){allsame=allsame+1;}
        }
  if (allsame==AttribLength+1){count=count+1;
  /*
      Serial.print(" at x=");Serial.print(x);Serial.print("  found id number:");Serial.println(count);Serial.print(" Next 20 data are  <");
          for (int show=0;show<=20;show++) { Serial.print(char(data[x+Y+show]));}
      Serial.println();
  */
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


extern void Picture();

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

void ParseSwitchList(byte Switch, byte* payload, unsigned int Length){
  String Result;
     // allow processor to do other things?
         LOCO_id[Switch]=Attrib(1,"\" id=\"",payload,Length);  // {" id="}
         Result=Attrib(1," bus=\"",payload,Length);
         SW_bus[Switch]=Result.toInt();
         Result=Attrib(1," addr1=\"",payload,Length);
         SW_addr[Switch]=Result.toInt();
     SwitchNumbers=Switch+1;
     if (SwitchNumbers>=MAXLOCOS-1) {SwitchNumbers=MAXLOCOS-1;}
   Serial.print("Attributes for <");Serial.print(Switch);Serial.print("> <");Serial.print(LOCO_id[Switch]);
   Serial.print(">  BUS:(==Rocnode)");Serial.print (SW_bus[Switch]);
   Serial.print(" addr:");Serial.print (SW_addr[Switch]);

     Serial.println(" ");
   }
 

#define Recipient_Addr  1   //  use with SetWordIn_msg_loc_value(sendMessage,Recipient_Addr,data  , or get sender or recipient addr  
#define Sender_Addr 3       //  use with SetWordIn_msg_loc_value(sendMessage,Sender_Addr,data   

uint8_t ROC_netid;
uint16_t ROC_recipient;
uint16_t ROC_sender;
uint8_t  ROC_group;
uint8_t  ROC_code;
uint8_t ROC_len;
uint8_t ROC_Data[200];


//

char*  Show_ROC_MSG(uint8_t Message_Length) {
char DebugMessage[128];
  if (Message_Length >= 1) {
    DebugMessage[0] = 0;
    strcat(DebugMessage, " NetID:");
    snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_netid);
    strcat(DebugMessage, " Rec:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_recipient);
    strcat(DebugMessage, " Sdr:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_sender);
    strcat(DebugMessage, " Grp:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_group);
    strcat(DebugMessage, " Code[");
    if ((ROC_code & 0x60) == 0) {
      strcat(DebugMessage, "Req]:");
    }
    if ((ROC_code & 0x60) == 0x20) {
      strcat(DebugMessage, "Evt]:");
    }
    if ((ROC_code & 0x60) == 0x40) {
      strcat(DebugMessage, "Rpy]:"); //// add request event reply then code.. (5 bits)
    }
    snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, (ROC_code & 0x1F));
    for (byte i = 1; i <= ROC_len; i++) {
      strcat(DebugMessage, " D"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, i);
      strcat(DebugMessage, "="); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_Data[i]);
    }

    //Serial.print(DebugMessage);
  } return DebugMessage;
}
void SetWordIn_msg_loc_value(uint8_t* msg, uint8_t firstbyte, int value) {
  msg[firstbyte + 1] = value % 256; //low byte
  msg[firstbyte] = value / 256; // order is high first then low
}
int getTwoBytesFromMessageHL( uint8_t* msg, uint8_t highloc) {
  return msg[highloc + 1] + msg[highloc] * (256);
}
int IntFromPacket_at_Addr(uint8_t* msg, uint8_t highbyte) { // example IntFromPacket_at_Addr(recMessage,Recipient_Addr))
  return msg[highbyte + 1] + msg[highbyte] * 256;
}
void dump_byte_array(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  }

String ThisID;
extern int switchindex;
extern int locoindex;

void MQTTFetch (char* topic, byte* payload, unsigned int Length) { //replaces rocfetch
  // do a check on length matching payload[7] ??
char PayloadAscii[100];
unsigned int y;

for (int i=0; i<=100;i++){PayloadAscii[i]=char(payload[i]);}

      Message_Decoded = false;
//  Do Clock synch   
  if ((strncmp("rocnet/ck", topic, 9) == 0)) { //==0 if n bytes are the same
     hrs = payload[12];
     mins = payload[13];
     secs = payload[14]; 
     Message_Decoded = true;
     delay(subIPL);
     DebugSprintfMsgSend( sprintf ( DebugMsg, " IPaddr .%d  Time Synchronised. ",subIPL));
    }
  
 if ((strncmp("rocnet/ps", topic, 9) == 0)) { //==0 if n bytes are the same this is a "Programming stationary" message
    
 //start decode     
    ROC_netid = payload[0];
    ROC_recipient = IntFromPacket_at_Addr(payload, Recipient_Addr); //1
    ROC_sender = IntFromPacket_at_Addr(payload, Sender_Addr);       //3
    ROC_group = payload[5];
    ROC_code = payload[6];
    ROC_len = payload[7];
    for (byte i = 1; i <= ROC_len; i++) {
      ROC_Data[i] = payload[7 + i];
    }

// what have we got ?
  
   // will use  SW_addr[switchindex])// Switch address   
   // LeftIndexPos = ;
   // RightIndexPos = ;
   // we should have asked for data about just 1 address from the bus(rocnode) we are interested in
   if (IntFromPacket_at_Addr(payload,Sender_Addr)==SW_bus[switchindex]){ // Serial.print("this is the correct sender/node");
   if (ROC_Data[1]==SW_addr[switchindex]){ // correct addr/pin
      Serial.println();Serial.println( " Correct Bus and Addr for Switch: getting positions");// check this is the data addr we want
      LeftIndexPos =IntFromPacket_at_Addr(ROC_Data,2);
      RightIndexPos =IntFromPacket_at_Addr(ROC_Data,4);
      for (byte x = 1; x <= 8; x++) {  SW_all[x]=ROC_Data[x];} //store the lot so we can send it back with modifified on and off pos from menu
       Message_Decoded = true;                                
       }}
   // return; 

    Message_Decoded = true; // but turn off debug print outputs in the final version
   if   ((!Message_Decoded)) {
   Serial.print(" Sender");Serial.print( IntFromPacket_at_Addr(payload,Sender_Addr));
   Serial.print(" Recipient");Serial.print( IntFromPacket_at_Addr(payload,Recipient_Addr));
   Serial.print(" group");Serial.print( ROC_group);
   Serial.print(" code");Serial.print( ROC_code);
   //Serial.print(" Looking for addr:");Serial.print(SW_addr[switchindex]);
   Serial.println("  ");  
                             }
       // for test purposes, if not decoded,  it  must be something else so print for debugging....
     
          if   ((!Message_Decoded)) { Serial.print("Unknown rocnet/ps MSG  is <"); Serial.print(Show_ROC_MSG(Length)); Serial.println(">");}
   } // if ((strncmp("rocnet/ps", 

   
// is it a service info?
 if ((strncmp("rocrail/service/info", topic, 20) == 0)) {
     /* if (strlen(Attrib(1,"\" id=\"",payload,Length))>=1){;  // {does it contain " id="}
      if ((ParseIndex<=MAXLOCOS)&& (!AllDataRead)) { Serial.print(F(" Found switch properties list  <"));Serial.print(ParseIndex);Serial.println(">");
      ThisID=Attrib(1,"\" id=\"",payload,Length); 
      if (ThisID==LOCO_id[switchindex]){Serial.print("this is a response for somethng we are working with at the moment");}
      else {
           if (ThisID==LOCO_id[0]){Serial.print(" Read this data before "); AllDataRead=true;    }
           else{ParseSwitchList(ParseIndex,payload,Length); ParseIndex++;}
      } 
          Message_Decoded = true;}
        } 
        */
  if (strlen(Attrib(1,"<lc id=\"",payload,Length))>=1){;  // {does it contain "<lc id="       
   if ((ParseIndex<=MAXLOCOS)&& (!AllDataRead)){ Serial.print(F(" Found loco properties list  <"));Serial.print(ParseIndex);Serial.println(">");
   ThisID=Attrib(1,"<lc id=\"",payload,Length);
   if (ThisID==LOCO_id[locoindex]){Serial.print("this is a response for somethng we are working with at the moment");
                                     }
   else {
         if (ThisID==LOCO_id[0]){Serial.print(" Read this loco before - switching off ParseLoco- "); //This is only triggered if the lcprops list is somehow re-triggered and we read again loco[0]
                                        AllDataRead=true;}
                else{ParsePropsLocoList(ParseIndex,payload,Length); ParseIndex++;}
             }  
       Message_Decoded = true;}
       } 
        
 }
// known unimportant (to me) messages
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
 if(!Message_Decoded){
    Serial.println();
    Serial.print(F("--Service Info Message- Length:"));
    Serial.print(Length);Serial.print(F("  ="));
    //unsigned int y;
    y= MQTT_MAX_PACKET_SIZE;
    Serial.print((100*Length)/MQTT_MAX_PACKET_SIZE);Serial.println(F("% msg size used"));
    Serial.print("begins:"); for (byte i = 1; i <= 60; i++) {
      Serial.print(PayloadAscii[i]);} 
      Serial.println();
    
    //return;
 
      }
       
  

 Message_Decoded=true;  // switch off printout 
// of not decoded, then it  must be something else so print for debugging....

//debug printing
if (!Message_Decoded){
 bool NewLine;
 Serial.println(F("**  MQTT FETCHED - but Not decoded"));
 Serial.print("**  Topic <");
 for (byte i = 0; i <= strlen(topic); i++) {Serial.print(topic[i]);}
  Serial.println(">");
  
 Serial.print("**  Payload  length<");Serial.print(Length);Serial.println(">");
 NewLine=false;
 for (long i = 0; i <= Length; i++) {Serial.write(char(payload[i]));
 if ((i%60)==0){NewLine=true;} 
 if(NewLine && payload[i]==32){Serial.println();NewLine=false;}
 }
  Serial.println(">");
 Serial.println("---------------------");
Message_Decoded = true;
}// end of debug printing...
  
 
} 
                                             
void MQTTRocnetSend (char* topic, uint8_t * payload) { //replaces rocsend
  uint8_t Length;
  Length = payload[7] + 8;
  //  Serial.println();
  //  Serial.print("*MQTT Posting [");
  //  Serial.print(topic);
  //  Serial.print("] ");
  //  dump_byte_array(payload, Length);
  //  Serial.println(">");
  client.publish(topic, payload, Length);
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
//extern uint16_t HandControlID;


void DebugMsgSend (char* topic, char* Debug_Payload) { // use with mosquitto_sub -h 127.0.0.1 -i "CMD_Prompt" -t debug -q 0
  char DebugMsgLocal[127];
    char DebugMsgTemp[127];
  int cx;


  cx= sprintf ( DebugMsgTemp, " Throttle:  Msg:%s",   Debug_Payload);

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
      cx= sprintf ( DebugMsgLocal, "  Time %d:%d:%ds Msg TOO Big to print",   hrs, mins, secs);
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
   char Msg[80];
   char ClientName[80];
  char myName[15] = "Switch Tool:";
  sprintf(ClientName, "%s%i", myName, wifiaddr);
 
  sprintf(Msg, "to %i:%i:%i:%i", mosquitto[0],mosquitto[1],mosquitto[1],mosquitto[3]);
  // Loop until we're reconnected 
  //digitalWrite (LED_BUILTIN , SignalON) ; ///   turn on
  PrintTime(" Attempting MQTT (re)connection. Attempt #");
  Serial.println(connects);
  while (!client.connected()) {
   
    Serial.print(" trying:");
    Serial.print(mosquitto);Serial.print("  ");Serial.print(ClientName);
    Serial.println("  ");
    // Attempt to connect

    if (client.connect(ClientName)) {
      Serial.println();
      // can advise this node is connected now:
       DebugSprintfMsgSend( sprintf ( DebugMsg, "%s Connected at:%d.%d.%d.%d",ClientName,ip0,ip1,subIPH,subIPL));
      
      // ... and now subscribe to topics  http://wiki.rocrail.net/doku.php?id=rocnet:rocnet-prot-en#groups
      
      client.subscribe("rocrail/service/info", 1 ); // server information
      client.subscribe("rocnet/ck", 1 ); // Clock synch  information
      client.subscribe("rocnet/ps", 1 ); // Data from the rocnode
     // client.subscribe("rocrail/service/client", 1 ); // an we see client data? test..       
     // delay(100);

     // no eeprom used in this sketch  EPROM_Write_Delay = millis();
     
    } else {
      Serial.print(" failed to find broker ");
      //Serial.print(client.state()); 
     // 
    delay(100);
    client.setServer(mosquitto, 1883);   // Hard set port at 1833
      Serial.println(" try again ...");
      delay(100);
      digitalWrite (LED_BUILTIN , SignalOFF) ; ///   turn OFF
    }
  }
}



