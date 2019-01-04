
#include "MQTT.h"
#include <Arduino.h> //needed 
//  #include "Directives.h";
#define SignalON LOW  // defined so I can change the "phase of the SignalLED" easily.
#define SignalOFF HIGH


// put these changes in PubSubClient.h at or around line 17 or so..
//        #define MQTT_MAX_PACKET_SIZE 10000   // lclist is LONG...!
//(And, if using RSMB and not Mosquitto..)
//        #define MQTT_VERSION MQTT_VERSION_3_1 //Rocrail needs to use V 3_1 not 3_1_1 (??)
#include <PubSubClient.h>




#include <WiFiClient.h>
WiFiClient espClient;

PubSubClient client(espClient);

extern IPAddress mosquitto;
extern String NameOfThisThrottle;
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

String FunctionName[N_Functions+1];
bool FunctionState[N_Functions+1];
int FunctionTimer[N_Functions+1];
String LOCO_id[MAXLOCOS+1];
String SW_id[MAXLOCOS+1];


int SW_bus[MAXLOCOS+1];
int SW_addr[MAXLOCOS+1];
extern int switchindex;
extern int RightIndexPos;
extern int LeftIndexPos;
extern byte SW_all[9];

int Count(unsigned int start,char* id, byte* data,unsigned int Length) {// to find string id in data and count  (max 65k)
 int count;int allsame;
 unsigned int id_Length;
 bool same;
 count=0;
 id_Length = strlen(id)-1;
// Serial.println();Serial.print("length of string \"");Serial.print(id);// note  \"
// Serial.print("\" is:");Serial.println(id_Length);
//for (int y=0;y<=(id_Length);y++){
//  Serial.print("id --<");Serial.print(y);Serial.print("> ={");Serial.print(id[y]);Serial.println("}");
// }
// Serial.print("char(data) --<");
//for (int y=0;y<=50;y++){
//  Serial.print(char(data[y]));
//}Serial.println();
//Serial.print("starting count of how many <");Serial.print(id);Serial.println(">'s");
 for (unsigned int x=start ; x<= Length-id_Length-1; x++){
  allsame=0;
  for (int y=0;y<=(id_Length);y++){
    if (id[y]==char(data[x+y])){allsame=allsame+1;}
        }
  if (allsame==id_Length+1){count=count+1;
  /*
      Serial.print(" at x=");Serial.print(x);Serial.print("  found id number:");Serial.println(count);Serial.print(" Next 20 data are  <");
          for (int show=0;show<=20;show++) { Serial.print(char(data[x+Y+show]));}
      Serial.println();
  */
  }
   
 }
 return count;
}
  // #define _attribdebug
 // #define _attribdebugc
 // #define _attribdebugplus
 extern const int MaxAttribSize=35 ;
 extern char FoundAttrib[MaxAttribSize];
 
char* ParseforAttribute(String First, String AttributeName, char End, byte* data,unsigned int Length){; // to find string First then string AttributeName and get chars after that up to Char End in data 
int count;int allsame;int Firstcount; int Y; int Z;
 unsigned int AttributeName_Length;
 unsigned int First_Length;
 bool FirstFound;
 bool SentFirstMsg;bool SentSecondMsg;
  First_Length = (First.length())-1;
  AttributeName_Length = (AttributeName.length())-1;
  count=0;
  SentFirstMsg=false;
  SentSecondMsg=false; 
  for (int fill=0; fill<=MaxAttribSize;fill++){FoundAttrib[fill]=0;  } // fill with null
  
 // for debug 
 #ifdef _attribdebugplus 
  Serial.println();Serial.print("starting search for (first) entry  of <");Serial.print(First);Serial.print("> then < ");Serial.print(AttributeName);Serial.println(">  ");
 #endif 
 FirstFound=false;  
    for (unsigned int x=0 ; x<= Length-AttributeName_Length-20; x++){
       allsame=0;
       Firstcount=0;

// at this x offset, see if we have first text to see if its present at data[x]
       for (int w=0;w<=(First_Length);w++){
                         #ifdef _attribdebugplus        // sends 150 chars from the  "x>=" onwards. to manually check we have the data we are looking for 
                              //    if  ((x>=2532)&&!SentSecondMsg){SentSecondMsg=true; 
                               //   Serial.print("MSG @");Serial.print(x);Serial.print(" is {");
                               //       for (int te=0;te<=150;te++){
                               //                Serial.print(char(data[x+te]));
                                 //                    }Serial.println("}");}
                         #endif
            if (First[w]==char(data[x+w])){Firstcount=Firstcount+1;}//loop checks each byte of AttributeName
                if (Firstcount==First_Length+1){
                    FirstFound=true;
                          Z=x; // store offset in case we need it later
                          #ifdef _attribdebugplus        
                                  if  (!SentFirstMsg){SentFirstMsg=true; 
                                  Serial.print("Found occurance of First text at char<");Serial.print(x);Serial.println(">   ");
                                                     }
                         #endif
                                              } // we found First text 
                                             } // end of w loop
                // now look on for textAttributeNamefrom this point on
                
  if (FirstFound){  
     #ifdef _attribdebugplus  
           Serial.print(char(data[x+Z]));
           #endif
    allsame=0;            
           for (int y=0;y<=(AttributeName_Length);y++){  Y=y; 
           if (AttributeName[y]==char(data[x+y])){allsame=allsame+1;
           #ifdef _attribdebugplus  
       //    Serial.print("!");
           #endif
                                                  }// all the same so far
                                       }//y loop checks each byte of AttributeName
                if (allsame==AttributeName_Length+1){count=count+1;} // count 
                if (count==1){  // found  nth occurrence of AttributeName
         // for debug
          #ifdef _attribdebugplus        
                     Serial.print("found Second text at char<");Serial.print(x);Serial.print(">  is<");
          #endif
               for (int z=0 ;z<=MaxAttribSize;z++) { 
                   //FoundAttrib[z]=char(data[x+Y+z+1]);
                   if (data[x+Y+z+1]==End){FoundAttrib[z]=0;  // end of data with char End eg, 34 == (")
                   for (int fill=z; fill<=MaxAttribSize;fill++){FoundAttrib[fill]=0;  } // not needed with filled string with '0' ?
                    // for debug  
                     #ifdef _attribdebugplus 
                           Serial.println(">");
                     #endif
                     //delayMicroseconds(3);// a short delay seems essential if char* foundattrib is defined INSIDE the function..3us is recomended minimum 
                   return FoundAttrib;}
                else{FoundAttrib[z]=char(data[x+Y+z+1]);
                  // for debug 
                   #ifdef _attribdebugplus  
                      Serial.print(char(data[x+Y+z+1]));
                   #endif  
                  }
         }// z loop
    } //if nth occurance  same
  }
   } //x loop
 return FoundAttrib;
}




char* Attrib(char End,String id, byte* data,unsigned int Length) {// to find string id in data and count  (max 65k)
 int Nth=1;
 int count;int allsame;int Y;
 unsigned int id_Length;
 //const int MaxAttribSize =35;
 //char FoundAttrib[MaxAttribSize];// FoundAttrib= will be the attributes found by this function;
 
 // char* FoundAttrib;// FoundAttrib= will be the attributes found by this function;
  // FoundAttrib="";// instantiate string or get problems later!
 
  id_Length = (id.length())-1;
  count=0;
 // for debug 
 #ifdef _attribdebug 
  Serial.println();Serial.print("starting search for <");Serial.print(id_Length+1);Serial.print( "> bytes of entry <");Serial.print(Nth);Serial.print("> for <");Serial.print(id);Serial.print("> .in.. ");
 #endif 
  
  for (int fill=0; fill<=MaxAttribSize;fill++){FoundAttrib[fill]=0;  } // fill with null
  
  // for debug  
 //for (unsigned int x=0 ; x<= Length-id_Length-20; x++){
 // Serial.print(char(data[x])); }
 // Serial.println("");
  
  for (unsigned int x=0 ; x<= Length-id_Length-20; x++){
       allsame=0;
       for (int y=0;y<=(id_Length);y++){  Y=y; if (id[y]==char(data[x+y])){allsame=allsame+1; 
                                  //Serial.print(allsame); 
                                       }// all the same so far
                                       }//y loop checks each byte of id
  
        if (allsame==id_Length+1){count=count+1;} // count 
            if (count==Nth){
         // for debug
          #ifdef _attribdebug        
                     Serial.print("found Nth text at char<");Serial.print(x);Serial.print(">  is<");
          #endif
               for (int z=0 ;z<=MaxAttribSize;z++) { 
                   //FoundAttrib[z]=char(data[x+Y+z+1]);
                   if (data[x+Y+z+1]==End){FoundAttrib[z]=0;  // end of data with ascii dec 34 == (")
                   for (int fill=z; fill<=MaxAttribSize;fill++){FoundAttrib[fill]=0;  } // not needed with filled string with '0' ?
                    // for debug  
                     #ifdef _attribdebug 
                           Serial.println(">");
                     #endif
                     //delayMicroseconds(3);// a short delay seems essential if char* foundattrib is defined INSIDE the function..3us is recomended minimum 
                   return FoundAttrib;}
                else{FoundAttrib[z]=char(data[x+Y+z+1]);
                  // for debug 
                   #ifdef _attribdebug  
                      Serial.print(char(data[x+Y+z+1]));
                   #endif  
                  }
         }// z loop
    } //if nth occurance  same
 
   } //x loop
 return FoundAttrib;
}


extern void Picture();

void ParsePropsLocoList(byte loco, byte* payload, unsigned int Length){
       // propslist gives just one loco at a time so the Attrib length is 1 every time. but will be stored in index [loco]
      LOCO_id[loco]=Attrib('\"',"<lc id=\"",payload,Length);   
      //Serial.print("Name read for loco id[");Serial.print(loco);Serial.print("] <");Serial.print(LOCO_id[loco]);
      //Serial.println("> ");
   }

void ParseLocoFunctionsList(byte loco, byte* payload, unsigned int Length){
  String LookFor; String SecondText; String FirstText; 
  int index;
  index=0;
     Serial.print("Functions reading for loco id[");Serial.print(loco);Serial.print("] <");Serial.println(LOCO_id[loco]);
     for (index=0;index<=16;index++){
      LookFor="<fundef fn=\"";
      LookFor=LookFor+index;
      LookFor+="\""; 
      FirstText=" text=\"";
      SecondText=" timer=\"";
      FunctionName[index] = ParseforAttribute(LookFor,FirstText,'\"',payload,Length);
      FunctionTimer[index] = atoi(ParseforAttribute(LookFor,SecondText,'\"',payload,Length));
      //Serial.print("fn name");Serial.print(" <");Serial.print(FunctionName[index]); Serial.print("> Timer:"); Serial.println(FunctionTimer[index]);
      //Serial.print("fn name");Serial.print(" <");Serial.print(FunctionName[index]); Serial.print("> "); Serial.print("timer");Serial.print(" <");Serial.print(FunctionTimer[index]); Serial.print("> ");
      }
      // Serial.println("> ");
   }
void ParseSwitchList(byte Switch, byte* payload, unsigned int Length){
  String Result;
     // allow processor to do other things?
         SW_id[Switch]=Attrib('\"',"\" id=\"",payload,Length);  // {" id="}
         Result=Attrib('\"'," bus=\"",payload,Length);
         SW_bus[Switch]=Result.toInt();
         Result=Attrib('\"'," addr1=\"",payload,Length);
         SW_addr[Switch]=Result.toInt();
     SwitchNumbers=Switch+1;
     if (SwitchNumbers>=MAXLOCOS-1) {SwitchNumbers=MAXLOCOS-1;}
   Serial.print("Attributes for <");Serial.print(Switch);Serial.print("> <");Serial.print(SW_id[Switch]);
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
extern char DebugMessage[128];
char*  Show_ROC_MSG(uint8_t Message_Length) {
  //char DebugMessage[128];
    if (Message_Length >= 1) {
    DebugMessage[0] = 0;
    strcat(DebugMessage, " Length:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, Message_Length);
    strcat(DebugMessage, " NetID:");    snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_netid);
    strcat(DebugMessage, " Rec:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_recipient);
    strcat(DebugMessage, " Sdr:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_sender);
    strcat(DebugMessage, " Grp:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_group);
    strcat(DebugMessage, " DLen:"); snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, ROC_len);
    strcat(DebugMessage, " Code[");
    if ((ROC_code & 0x60) == 0) { strcat(DebugMessage, "Req]:");   }
    if ((ROC_code & 0x60) == 0x20) { strcat(DebugMessage, "Evt]:");}
    if ((ROC_code & 0x60) == 0x40) { strcat(DebugMessage, "Rpy]:"); //// add request event reply then code.. (5 bits)
                                    }
                                    snprintf(DebugMessage, sizeof(DebugMessage), "%s%d", DebugMessage, (ROC_code & 0x1F));
    strcat(DebugMessage, " Data:"); 
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
String ThisSpeed;
extern int switchindex;
extern int locoindex;
extern int speedindex;
extern int MenuLevel;


void MQTTFetch (char* topic, byte* payload, unsigned int Length) { //replaces original rocfetch
  // do a check on length matching payload[7] ??
  char PayloadAscii[101];
  unsigned int y;
  int cx;
  char DebugMsgLocal[127];
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
// end clock synch

// Rocnet messages are included Because a variant of this code exists (Client Tool) that allows modification of Servo positions
//   a "Programming stationary" message
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
   // for test purposes, if not decoded,  it  must be something else so print for debugging....
     Message_Decoded = true;     
    if   ((!Message_Decoded)) { Serial.print("Unknown rocnet/ps MSG Length<");Serial.print(Length);Serial.print("> is "); Serial.print(Show_ROC_MSG(Length));}
   } // if ((strncmp("rocnet/ps", 
   //end a "Programming stationary" message

 if ((strncmp("rocrail/service/client", topic, 22) == 0)) {
  Message_Decoded = true;// I ignore client messages for now  
      }
// is it a service info message?
if ((strncmp("rocrail/service/info", topic, 20) == 0)) {
    if (strlen(Attrib('\"',"<lc ",payload,Length))>=1){ThisID=Attrib('\"',"id=\"",payload,Length);
    //Serial.println(ThisID); // added for tests
    }
    
   bool same; 
   String _Show; 
   String _Active;
  if ((!Message_Decoded)&&(strlen(Attrib('\"',"<lc id=\"",payload,Length))>=1)&&(strlen(Attrib('\"',"prev_id=\"",payload,Length))>=1)){ // {does it contain "<lc id="  and "prev_id=" as well is should only capture lcprops messages?
     // this is may be data about a new loco ?
          // Serial.print("This ID is <");Serial.print(ThisID);Serial.print("> Loco list is ");
          // for (int check=0; check<=5 ;check++) { Serial.print("; [");Serial.print(check);Serial.print("] <");Serial.print(LOCO_id[check]);Serial.print("> ");}
          // Serial.println("");
           same=false;
                  for (int check=0; check<=MAXLOCOS ;check++) { if ((ThisID==LOCO_id[check])) { same=true;}}
                                                    // get to here with same=false if this is a new loco
                       if (!same){// NEW ...added check for show="true";  and active="true"
                        _Show=ParseforAttribute("<lc id=\"","show=\"",'\"',payload,Length);
                        _Active=ParseforAttribute("<lc id=\"","active=\"",'\"',payload,Length);
                        //Serial.print (" found<");Serial.print(_Show);Serial.println ("> ");
                        
                        if ((_Show=="true")&&(_Active=="true")){
                         // Serial.print (" got here<");Serial.print(_Show);Serial.println ("> ");
                                                    ParsePropsLocoList(ParseIndex,payload,Length);
                                                    // LocoNumbers++;
                                                     if (LocoNumbers>=MAXLOCOS-1) {LocoNumbers=MAXLOCOS-1;} // overwrite the last number if too many locos
                                                   // DebugSprintfMsgSend(sprintf(DebugMsg,"New Loco found..<%s> ",ThisID.c_str()));
                                                    DebugSprintfMsgSend(sprintf(DebugMsg," Loco list contains <%d> Locos",ParseIndex+1));
                                                    ParseIndex++;
                                                    LocoNumbers=ParseIndex;
                       }
                                                    }
                                                    else {// update properties for current 
                                                    ParseLocoFunctionsList(locoindex,payload,Length);  
                                                    Message_Decoded = true; 
                                                      }                   
                                                    
           Message_Decoded = true; }  // end of new loco test

           
// function change? in fn message..          
int FN_seen;
String FN_ascii;
String FN_state_string; 
String LocoIDForFnChanged;
bool FN_attribute; 
String MSG;
// format #1
MSG="<fn fnchanged=\"";
if ((strlen(Attrib('\"',MSG,payload,Length))>=1)){  
                 
              LocoIDForFnChanged=ParseforAttribute("<fn fnchanged","id=\"",'\"',payload,Length);
              if (LocoIDForFnChanged==LOCO_id[locoindex].c_str()){  //   Serial.print("~ For Me");
              FN_seen=atoi(Attrib('\"',"<fn fnchanged=\"",payload,Length));
              MSG=" f";
              MSG+=FN_seen;
              MSG+="=\"";
              FN_state_string=ParseforAttribute("<fn fnchanged",MSG,'\"',payload,Length);
              if (FN_state_string=="true"){FN_attribute=true;}else{FN_attribute=false;}
              FunctionState[FN_seen]=FN_attribute;
                  //  Serial.print(F(" Function changed seen "));
                  //  Serial.print("  F<");Serial.print(FN_seen);
                  //  Serial.print("> fnchange_state:");Serial.println(FN_state_string); 
              }
              
              Message_Decoded = true;// we understand this message
             }
// format #2
MSG="<fn id=\"";
if ((!Message_Decoded)&&(strlen(Attrib('\"',MSG,payload,Length))>=1)){  
              LocoIDForFnChanged=Attrib('\"',MSG,payload,Length);
              if (LocoIDForFnChanged==LOCO_id[locoindex].c_str()){Serial.print(" Me");
              FN_seen=atoi(ParseforAttribute(MSG,"\" f",'\"',payload,Length));
              MSG=" f";
              MSG+=FN_seen;
              MSG+="=\"";
              FN_state_string=Attrib('\"',MSG,payload,Length);
              if (FN_state_string=="true"){FN_attribute=true;}else{FN_attribute=false;}
              FunctionState[FN_seen]=FN_attribute;
                    Serial.print(F(" Fn change seen "));
                    Serial.print("  F<");Serial.print(FN_seen);
                    Serial.print("> fnchange_state:");Serial.println(FN_state_string); 
              }
                            Message_Decoded = true;// we understand this message
             }
// function change seen in <lc message?             
//if ((strlen(Attrib('\"',"<lc ",payload,Length))>=1)    &
//            (strlen(Attrib('\"',"id=\"",payload,Length))>=1)){  
//              Message_Decoded = true;
//             }

// speed changed ? (also checks lights status)     
// format #3
MSG="<lc id=\"";
//if ((!Message_Decoded)&&(strlen(Attrib('\"',MSG,payload,Length))>=1)){  
  if ((strlen(Attrib('\"',MSG,payload,Length))>=1)){  
              LocoIDForFnChanged=Attrib('\"',MSG,payload,Length);
              if (LocoIDForFnChanged==LOCO_id[locoindex].c_str()){
                ThisSpeed=Attrib('\"',"V=\"",payload,Length);
                  if(strlen(Attrib('\"',"dir=\"false\"",payload,Length))>=1){
                                             speedindex= -ThisSpeed.toInt();}
                                        else {speedindex= ThisSpeed.toInt();}
                FN_state_string=ParseforAttribute("<lc id=\"","fn=\"",'\"',payload,Length);  
                      if (FN_state_string=="true"){FN_attribute=true;}else{FN_attribute=false;}
                      FunctionState[0]=FN_attribute;
               Serial.print(F("Speed change message for Me"));  Serial.print(F(" Velocity :"));Serial.print(speedindex);  Serial.print(F(" Lights status :"));Serial.println(FN_state_string); 
                     } // this message was for me
               Message_Decoded = true;// we understand this message
             }


// release throttle.. DOES NOTHING apart from set menu level to zero as a test!!Because we should have more than one loco in the list
if (strlen(Attrib('\"',"<lc id=\"",payload,Length))>=1){
      ThisID=Attrib('\"',"<lc id=\"",payload,Length);  // get the name now..
      if (strlen(Attrib('\"',"cmd=\"release\"",payload,Length))>=1){ // is this the release message 
         //Serial.println("-");
        // Serial.print("Throttle Release for ");
        // Serial.println(ThisID);
          if ((ThisID==LOCO_id[locoindex])){
         Serial.print(F("Throttle Release seen for me!"));
          //DebugMsgSend("debug","Throttle control released");
          //DebugSprintfMsgSend(sprintf(DebugMsg," control of %s released",ThisID.c_str()));
          //ThisID="This is an unlikely name";
          //LOCO_id[0]=ThisID;
          //LocoNumbers=0;
          //ParseIndex=0;
          //AllDataRead=false;
          MenuLevel=0;
                 }}
          Message_Decoded = true;
       }
      
        
    }// end of service info message checks , including release loco throttle
 
                                 
// other known but unimportant (to me in this code) messages
     if (strncmp("<exception text=",PayloadAscii,16)==0){
         Message_Decoded = true; 
         }  
     if (strncmp("<clock divider=",PayloadAscii,15)==0){
      //Serial.print(F("Clock msg<"));
         Message_Decoded = true; }
     if (strncmp("<program modid=",PayloadAscii,15)==0){
         Message_Decoded = true; }
     if (strncmp("<sw id=",PayloadAscii,7)==0){
         Message_Decoded = true; }
     if (strncmp("<model cmd=\"lcprops\" ",PayloadAscii,20)==0){
         Message_Decoded = true; }
     // end of other known but unimportant (to me in this code) messages    
     
      //switch off debug message decode here
      Message_Decoded=true; 
           
 if(!Message_Decoded){
   if ((strncmp("rocnet", topic, 6) == 0)) {  Serial.println(""); Serial.print("Unknown rocnet MSG is topic<"); Serial.print(topic);Serial.print(">  <");Serial.print(Show_ROC_MSG(Length));}
       else{
    Serial.println();
    Serial.print(F("--Service  Message- Length:"));
    Serial.print(Length);Serial.print(F("  (truncated version shown)="));
    //unsigned int y;
    //MQTT_MAX_PACKET_SIZE should be around 10000;
    Serial.print((100*Length)/10000);Serial.print(F("% msg size used"));
    Serial.println(""); Serial.print(PayloadAscii);
    /*
    for (byte i = 0; i <= 120; i++) {
      Serial.print(PayloadAscii[i]);} 
    */
    Serial.println("");
       }
    //return;
 
      }
       
  
// of not decoded, then it  must be something else so print for debugging....
//Message_Decoded=true;  // switch off detailed printout 
// of not decoded, then it  must be something else so print for debugging....

//debug printing
if (!Message_Decoded){
 bool NewLine;
 Serial.println("");
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

void MQTTSend (String topic, String payload) { //replaces rocsend
  uint8_t Length;
  digitalWrite (LED_BUILTIN, HIGH) ;  /// turn On
  Length = payload.length();
  client.publish(topic.c_str(), payload.c_str(), Length);

}


//   ++++++++++ MQTT setup stuff   +++++++++++++++++++++
void  MQTT_Setup(void){
  client.setServer(mosquitto, 1883);   // Hard set port at 1833
  client.setCallback(MQTTFetch);
}


boolean MQTT_Connected(void){
  return client.connected();
}

void MQTT_Loop(void){
    client.loop(); //gets wifi messages etc..
}


void DebugMsgSend (String topic, String Debug_Payload) { // use with mosquitto_sub -h 127.0.0.1 -i "CMD_Prompt" -t debug -q 0
  char DebugMsgLocal[127];
    char DebugMsgTemp[127];
  int cx;

  cx= sprintf ( DebugMsgTemp, " Throttle<%s>  Msg:%s",NameOfThisThrottle.c_str(),Debug_Payload.c_str());
   // add timestamp to outgoing message
if ((hrs==0)&&(mins==0)){// not Synchronised yet..
  cx=sprintf(DebugMsgLocal," Time not synchronised yet %s",DebugMsgTemp);
   }
   else {cx=sprintf(DebugMsgLocal,"<%02d:%02d:%02ds> %s",hrs,mins,secs,DebugMsgTemp);
         }
//
    //Serial.printf("\n *Debug Message:%s Msg Length:%d \n",DebugMsgLocal,cx);
 //   Serial.printf("\n *Debug Message:%s  \n",DebugMsgLocal);
    Serial.printf("\n *Debug Message:%s  ",DebugMsgLocal);
 
    if ((cx <= 120)) {
      client.publish(topic.c_str(), DebugMsgLocal, strlen(DebugMsgLocal));
                     }
    if ((cx >= 120) && (strlen(Debug_Payload.c_str()) <= 100)) {
      cx= sprintf ( DebugMsgLocal, "MSG-%s-", Debug_Payload.c_str());
      client.publish(topic.c_str(), DebugMsgLocal, strlen(DebugMsgLocal));
                                          }// print just msg  line
    if (strlen(Debug_Payload.c_str()) >= 101) {
      cx= sprintf ( DebugMsgLocal, "  Time %d:%d:%ds Msg TOO Big to print",   hrs, mins, secs);
      client.publish(topic.c_str(), DebugMsgLocal, strlen(DebugMsgLocal));
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

void DebugSprintfMsgSend(int CX){ // allows use of Sprintf function in the "cx" location to send DebugMsg
  DebugMsgSend ("debug", DebugMsg);
  delay(5);
}
extern uint8_t wifiaddr;
extern void OLED_Display(char* L1,char* L2, char *L3);

void reconnect() {
   int cx;
   int cx1;
   int cx2;
   char MsgTemp[127];
   char  MsgBL[127];
   char  MsgML[127];
   char  MsgTL[127];
   
   char Msg[80];
   char ClientName[128];
  
  sprintf(ClientName, "%s%i", NameOfThisThrottle.c_str(), wifiaddr);  // build client  name from throttle name plus wifi address, this should result in  a unique identifier
 
  sprintf(Msg, "to %i:%i:%i:%i", mosquitto[0],mosquitto[1],mosquitto[1],mosquitto[3]);
  // Loop until we're reconnected 
  digitalWrite (LED_BUILTIN , SignalOFF) ; ///   turn on led 
 
  while (!client.connected()) { PrintTime(" Attempting MQTT (re)connection. Attempt #");
                                Serial.println(connects);
             // for debug     Serial.print("<");Serial.print(ClientName);Serial.print("> looking for MQTT broker @:");Serial.print(mosquitto); Serial.println("  ");
             // Messages to assist the user....  
                    cx1= sprintf (MsgTL, "%s",ClientName);  // The strange formatting here places the text for Oled disply in Char[127] defined arrays to avoid a compiler deprecation notification.
                    cx2= sprintf (MsgML, "looking for MQTT broker");  // its not strictly necessary, but I wanted to see if the deprecation warning could be avoided. 
                    cx= sprintf (MsgBL, "@ addr: %d:%d:%d:%d", mosquitto[0],mosquitto[1],mosquitto[2],mosquitto[3]);
                    OLED_Display(MsgTL,MsgML,MsgBL);
                   // OLED_Display("Looking for","a MQTT Broker @",MsgBL);  // using this simpler call works, but results in a deprecated complier error notice...
           // Attempt to connect
           if (client.connect(ClientName)) {// can advise this node is now connected 
                              // for debug   Serial.print("connected);  Serial.println();
                              DebugSprintfMsgSend( sprintf ( DebugMsg, "%s Connected at:%d.%d.%d.%d",ClientName,ip0,ip1,subIPH,subIPL));
                              // ... so now subscribe to topics  http://wiki.rocrail.net/doku.php?id=rocnet:rocnet-prot-en#groups
                                     client.subscribe("rocrail/service/info", 1 ); // server information
                                     client.subscribe("rocnet/ck", 1 ); // Clock synch  information
                                     client.subscribe("rocnet/ps", 1 ); // Data from the rocnode
                                     //client.subscribe("rocnet/#", 1 ); // Data from all the rocnet msgs
                                     client.subscribe("rocrail/service/client", 1 ); // so can we see client data? test..       
                                      // no eeprom used in this sketch so no delay needed for write delay EPROM_Write_Delay = millis();
                                            }  
                                            else {  // This is where we go if we are NOT connected
                                           //Serial.println("~ failed to find MQTT broker ");
                                           connects=connects+1;  // Count how many times we do this. So that a single miss to connect to the brokwr does not immediately trigger broker address incrementation
                                          if (connects>=5){  mosquitto[3] = mosquitto[3]+1; }
                                                  if (mosquitto[3]>=50){mosquitto[3]=3;}
                                                 // Serial.print(" trying MQTT <");Serial.print(mosquitto[3]);Serial.println("> ");  
                                           client.setServer(mosquitto, 1883);   // Hard set port at 1833
                                           delay(100);
                                           digitalWrite (LED_BUILTIN , SignalON) ; ///   turn ON
                                           delay(100);
                                           digitalWrite (LED_BUILTIN , SignalOFF) ; ///   turn ON
                                                }
                        }// end of while not connected
}



