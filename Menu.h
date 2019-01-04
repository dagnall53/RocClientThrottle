
  #include "Arduino.h"
// Throttle settings extern variables for display and selection stuff
extern int MenuLevel;
extern int locoindex;
extern int fnindex;
extern int speedindex;
extern bool directionindex;
extern int LocoNumbers;
//extern int y;
extern uint32_t ButtonPressTimer;
//extern uint32_t LcPropsEnabled;
extern byte ParseIndex;
extern bool AllDataRead;
extern void GetLocoList();
extern void GetLocoFunctions(int index);

extern String FunctionName[(N_Functions+1)];
extern bool FunctionState[(N_Functions+1)];
extern int FunctionTimer[(N_Functions+1)];
bool FunctionActive;
/*char* Str2Chr(String stringin){
  char* Converted;
  int cx;
  Converted="";
 // Serial.print(" Converting :");Serial.print(stringin);
   for (int i=0; i<=(stringin.length()+1);i++){ Converted[i]=stringin.charAt(i); }
 //  Serial.print(" to:");Serial.println(Converted);
   return Converted;
}
*/
void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
   // Go to: http://www.online-utility.org/image_converter.jsp
//Select MONO as Output format AND press “Select format” (Who has invented THAT UI???)
//Upload your image. I was succesful with a monochrome PNG file, whereas the XBM only resulted in an error.
//Upload the created MONO file to your ESP8266 and use it with a code like this:
   
   // display.drawXbm(20,20, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits); 
    display.drawXbm(0,0, Terrier_Logo_width, Terrier_Logo_height, Terrier_Logo); 
    
}

extern long ThrottlePosition;
extern bool Encoder_Timeout;

bool LastDir;
int LastLoco;
void SetLoco(int locoindex,int speedindex){
  bool Dir;
   char MsgTemp[200];
  int cx;
  int SpeedSelected;
  SpeedSelected=speedindex;

Dir=LastDir;
LastLoco=locoindex;
if (speedindex>=1){Dir=true;}
if (speedindex<=-1){Dir=false;}  // this set of code tries to ensure that when speed = 0 it uses the last direction set. 

 if (Dir) {LastDir=true; cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" dir=\"true\"  throttleid=\"%s\" />",LOCO_id[locoindex].c_str(),SpeedSelected,NameOfThisThrottle.c_str());}
      else{LastDir=false;cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" dir=\"false\"  throttleid=\"%s\" />",LOCO_id[locoindex].c_str(),abs(SpeedSelected),NameOfThisThrottle.c_str());}

  //Serial.print(LOCO_id[locoindex]);
  //Serial.print(" <");Serial.print(MsgTemp);Serial.println(">"); // to help with debug
  MQTTSend("rocrail/service/client",MsgTemp);
}


void Send_function_command(int locoindex,int fnindex, bool state){
  char MsgTemp[200];
  int cx;
        if (state){  cx=sprintf(MsgTemp,"<fn fnchanged=\"%d\" fnchangedstate=\"false\" id=\"%s\" f%d=\"false\" /> ",fnindex,LOCO_id[locoindex].c_str(),fnindex);}
          else      { cx=sprintf(MsgTemp,"<fn fnchanged=\"%d\" fnchangedstate=\"true\" id=\"%s\" f%d=\"true\" /> ",fnindex,LOCO_id[locoindex].c_str(),fnindex);}
 // command is of this general form.. <fn fnchanged="9" fnchangedstate="true" id="Test Board"  f9="true" 
          
// Serial.print(LOCO_id[locoindex]);Serial.println(" fn set<");Serial.print(MsgTemp);Serial.println(">");
  MQTTSend("rocrail/service/client",MsgTemp);
}

void Do_Function(int locoindex,int fnindex){
  // initially only f0 is toggle but now checks if timer is not zero
  if (FunctionTimer[fnindex]<=0) {
                FunctionState[fnindex]=!FunctionState[fnindex];
                Send_function_command(locoindex,fnindex,!FunctionState[fnindex]);
                FunctionActive=false;}
  else{FunctionState[fnindex]=true;
      Send_function_command(locoindex,fnindex,false);
      FunctionActive=true;
      }
}

extern void Picture();

void DoDisplay(int MenuLevel){
     //display.clear;
  String SpeedIndexString = String(speedindex);
  String FnIndexString= String(fnindex);
  String SpeedSelected;   
  String TopMessage = "---Select Loco---";
  String BottomMessage = "Selected:";
  String MSGText;
  BottomMessage += locoindex+1;
  BottomMessage += " of :";
  BottomMessage += (LocoNumbers);
  BottomMessage+= " available";

  switch (MenuLevel){
  
     case 0:  // top level
         display.setFont(ArialMT_Plain_10);
        // only display this if we actually have a loco list!
        if (LocoNumbers<=0){
            display.setFont(ArialMT_Plain_16);
            display.drawString(64,1,"No Loco");
            display.drawString(64,16,"List yet");}
       else{
            display.drawString(64,54,BottomMessage);
            display.setFont(ArialMT_Plain_10);
            display.drawString(64,0,TopMessage);
            if (locoindex>=1){
               display.setFont(ArialMT_Plain_10);
               display.drawString(64,16,LOCO_id[locoindex-1]);
                }
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.setFont(ArialMT_Plain_16);
            display.drawString(64,26,LOCO_id[locoindex]);
            if (locoindex<=LocoNumbers-2){
               display.setFont(ArialMT_Plain_10);
               display.drawString(64,40,LOCO_id[locoindex+1]);
               }       
           }   
  
      break;
 case 1: // selected loco, set speed
         //show loco
        if (LocoNumbers<=0){
             display.setFont(ArialMT_Plain_16);
             display.drawString(64,1,"No Loco");
             display.drawString(64,16,"selected");}
        else{
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_16);
          display.drawString(64,1,LOCO_id[locoindex]);
            }
       //show speed 
       SpeedSelected="";
       display.drawString(25,28," Speed:");
       display.drawString(90,28,SpeedIndexString);
    
      break;
      
case 2: // selected loco, set fn
 //show loco
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64,1,LOCO_id[locoindex]);
    
 //show fn
    display.setFont(ArialMT_Plain_10);
    display.drawString(64,20,"press for");
    if (FunctionName[fnindex]==""){MSGText="Fn:";MSGText+=fnindex;MSGText+=" ";}
    else {MSGText=FunctionName[fnindex];}
    MSGText+= "  ";
    display.setFont(ArialMT_Plain_16);
    display.drawString(64,34,MSGText);
    if (FunctionTimer[fnindex]==0){ MSGText=" State is:"; if (FunctionState[fnindex]){MSGText+="ON";}
                                                                                else{MSGText+="OFF";}
           display.setFont(ArialMT_Plain_10);  display.drawString(64,54,MSGText);                }  // Function timer is zero for toggle  so show current state
    
      
       
    
      break;
case 3: // selected loco, set something
     display.setTextAlignment(TEXT_ALIGN_CENTER);
     display.setFont(ArialMT_Plain_16);
     display.drawString(64,1,"--Level 3--");





      break;

 
 default:
 
      break; 
 
  
}//end  
display.display(); 
}

void ButtonDown(int MenuLevel){
 // Serial.print("DOWN");
switch (MenuLevel){
  
 case 0:  // top level
 locoindex=locoindex-1;
 break;
 case 1:  // top level
 speedindex=speedindex+1; // works better to increment with this button
 #ifdef Rotary_Switch
 if (!Encoder_Timeout){ speedindex=speedindex+9;}
 #endif
 SetLoco(locoindex,speedindex);// Rotary_Switch sets the loco in a different place
 
 break;
 case 2:
 fnindex=fnindex+1;
 if (fnindex>=16){fnindex=16;}
 break;
 default:
 break;
  

}


if (speedindex>=100){speedindex=100;}

if (locoindex <=0) {locoindex=0;}
if (locoindex>=LocoNumbers-1){locoindex=LocoNumbers-1;}
}

void ButtonUp(int MenuLevel){
  //Serial.print("UP");
switch (MenuLevel){
  
 case 0:  // top level
 locoindex=locoindex+1;
 break;
 case 1:  // level 1
 speedindex=speedindex-1;
 #ifdef Rotary_Switch
 if (!Encoder_Timeout){ speedindex=speedindex-9;}
 #endif
 SetLoco(locoindex,speedindex);// Rotary_Switch sets the loco in a different place
  break;
 case 2:
 fnindex=fnindex-1;
 if (fnindex<=0){fnindex=0;}
 break;
 default:
 break;
  
}


if (speedindex<=-100){speedindex=-100;}

if (locoindex <=0) {locoindex=0;}
if (locoindex>=LocoNumbers-1){locoindex=LocoNumbers-1;}
}


void ButtonRight(int MenuLevel){
  // nb Looks like you cannot change MenuLevel in a function that called with MenuLevel as a variable, presumably it sets internal variable only?



  
switch (MenuLevel){
 case 0:  // top level 
    GetLocoFunctions(locoindex);
  
      #ifdef Rotary_Switch
       ThrottlePosition=0; //ThumbWheel.write(0);
       // set Rotary_Switch to zero  
 
      #endif
      
      
 break;
 case 1:  // level 

 break;

 default:
  
 break;
}


}


void ButtonLeft(int MenuLevel){
 
  
switch (MenuLevel){
 case 0:  // top level 
 
 break;
 case 1:  // level 

 break;

 default:
  
 break;
}


}

void ButtonInactive(int MenuLevel){
if (FunctionActive){  
      FunctionActive=false;
      Send_function_command(locoindex,fnindex,true);
switch (MenuLevel){
  
 case 0:  // top level

 break;

 case 1:

   
 break;
 case 2:
  
  
break;
 default:
      
 break;
}
}
}


void ButtonSelect(int MenuLevel){ 
  char MsgTemp[200];
  int cx;
  switch (MenuLevel){
  
 case 0:  // top level//toot fn (index) 
    fnindex=2;
    Do_Function(locoindex,fnindex);
    FunctionActive=true;
       
      
 break;

 case 1:
 #ifndef Rotary_Switch
 if (speedindex==0){ 
    fnindex=2; 
    Do_Function(locoindex,fnindex);
    FunctionActive=true;
 }
 else{
   speedindex=0;
   SetLoco(locoindex,speedindex);}
#endif
    #ifdef Rotary_Switch
      if (speedindex==0){ Do_Function(locoindex,2); }//toot fn 2 (loco index) 
       else{  speedindex=0;ThrottlePosition=0;LastThrottlePosition=0; 
       //ThumbWheel.write(0); 
       SetLoco(locoindex,speedindex);}

#endif
   
 break;
 case 2:
  Do_Function(locoindex,fnindex);
  
break;
 default:
 break;
  
}
}



byte SW_all[9];
/*// Switch tool settings extern variables for display and selection stuff
extern int MenuLevel;
extern int switchindex;
extern int RightIndexPos;
extern int LeftIndexPos;
extern bool directionindex;
extern int SwitchNumbers;

//extern int y;
extern uint32_t ButtonPressTimer;
bool switchstate;
extern byte ParseIndex;

extern void SetWordIn_msg_loc_value(uint8_t* msg, uint8_t firstbyte, int value);

char* Str2Chr(String stringin){
  char* Converted;
  int cx;
  Converted="";
 // Serial.print(" Converting :");Serial.print(stringin);
   for (int i=0; i<=(stringin.length()+1);i++){ Converted[i]=stringin.charAt(i); }
 //  Serial.print(" to:");Serial.println(Converted);
   return Converted;
}

void drawImageDemo() {
    // Use GIMP to convert files..

   display.drawXbm(0,0, Terrier_Logo_width, Terrier_Logo_height, Terrier_Logo); 
   // display.drawXbm(34,14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits); 

}
void UpdateSetPositions(){
  uint8_t RocSendAddrCommand[15];
  RocSendAddrCommand[0]=0;
  RocSendAddrCommand[1]=0;
  RocSendAddrCommand[2]=SW_bus[switchindex];
  RocSendAddrCommand[3]=0;
  RocSendAddrCommand[4]=0;
  RocSendAddrCommand[5]=0x05; //group
  RocSendAddrCommand[6]=0x10; //code
  RocSendAddrCommand[7]=0x08; //length  
    for (byte x = 1; x <= 8; x++) { RocSendAddrCommand[7+x]=SW_all[x];} // paste other data as recieved..
  SetWordIn_msg_loc_value(RocSendAddrCommand, 7+2, LeftIndexPos);//re write bits we want to change..&+n because I should have done it on sw_all!
  SetWordIn_msg_loc_value(RocSendAddrCommand, 7+4, RightIndexPos);
  MQTTRocnetSend("rocnet/ps",RocSendAddrCommand);
}
extern char DebugMsg[127];
void SetSwitch(int switchindex,bool Throw){
   char MsgTemp[200];
  int cx;
  if (Throw)
  {cx=sprintf(MsgTemp,"<sw id=\"%s\" cmd=\"turnout\"  />",Str2Chr(LOCO_id[switchindex]),RightIndexPos);}
  else {cx=sprintf(MsgTemp,"<sw id=\"%s\" cmd=\"straight\"  />",Str2Chr(LOCO_id[switchindex]),RightIndexPos);}
  MQTTSend("rocrail/service/client",MsgTemp);
  DebugSprintfMsgSend( sprintf ( DebugMsg, " sending {%s}",MsgTemp));
  
  }
  
  extern void Picture();
void DoDisplay(int MenuLevel){
String BottomMessage = "Available Turnouts:";
BottomMessage += (SwitchNumbers);
display.setFont(ArialMT_Plain_10);
//display.drawProgressBar(0, 58,127,6, ((y*100)/64)); 
display.setTextAlignment(TEXT_ALIGN_CENTER);
if (SwitchNumbers>=1){
display.drawString(64,54,BottomMessage);}

switch (MenuLevel){
  
 case 0:  // top level
    display.setFont(ArialMT_Plain_10);
    display.drawString(64,1,"--- Select Switch ---");
if (SwitchNumbers<=0){
  Picture();
   display.setFont(ArialMT_Plain_10);
    display.drawString(64,27," Press to update");
    display.drawString(64,41,"List of Turnouts"); 
    display.setFont(ArialMT_Plain_16);
}
  else{  
    if (switchindex>=1){
     display.setFont(ArialMT_Plain_10);
     display.drawString(64,20,LOCO_id[switchindex-1]);
        }
     if (switchindex<=SwitchNumbers-1){
     display.setFont(ArialMT_Plain_10);
     display.drawString(64,44,LOCO_id[switchindex+1]);
        }    
    
     display.setFont(ArialMT_Plain_16);
    display.drawString(64,30,LOCO_id[switchindex]);
   
  }
      break;
 case 1: // selected Switch show Left pos

 if (SwitchNumbers<=0){
   display.setFont(ArialMT_Plain_16);
   display.drawString(64,1,"No Switch");
  display.drawString(64,16,"selected");}
   else
    {
     display.setFont(ArialMT_Plain_16);
     display.drawString(64,1,LOCO_id[switchindex]); 
     display.setFont(ArialMT_Plain_10);
     display.drawString(10,16,"Bus:");display.drawString(42,16,String(SW_bus[switchindex]));
     display.drawString(75,16,"Addr:");display.drawString(100,16,String(SW_addr[switchindex]));
    }
 //show setting  
    display.setFont(ArialMT_Plain_16);
    display.drawString(25,32,"Left:");
    display.drawString(60,32,String(LeftIndexPos));
   
      break;
      
case 2: // selected switch, set its right position

 if (SwitchNumbers<=0){
   display.setFont(ArialMT_Plain_16);
   display.drawString(64,1,"No Switch");
  display.drawString(64,16,"selected");}
   else
    {
     display.setFont(ArialMT_Plain_16);
     display.drawString(64,1,LOCO_id[switchindex]); 
     display.setFont(ArialMT_Plain_10);
     display.drawString(10,16,"Bus:");display.drawString(42,16,String(SW_bus[switchindex]));
     display.drawString(75,16,"Addr:");display.drawString(100,16,String(SW_addr[switchindex]));
    }
 //show setting  
    display.setFont(ArialMT_Plain_16);
    display.drawString(25,32,"Right:");
    display.drawString(60,32,String(RightIndexPos));
   


      break;
case 3: 
     display.setFont(ArialMT_Plain_16);
     display.drawString(64,1,"--Level 3--");





      break;

 
 default:
 
      break; 
 
  
}//end  
display.display(); 
}

extern bool SelectButtonState;
void ButtonDown(int MenuLevel){
  Serial.print("DOWN");
switch (MenuLevel){
  
 case 0:  // top level
 switchindex=switchindex-1;
 break;
 case 1:  // top level
 if(SelectButtonState){ LeftIndexPos=LeftIndexPos+2;}
 else  {LeftIndexPos=LeftIndexPos+15;}
 if (LeftIndexPos>=600){LeftIndexPos=600;}
  UpdateSetPositions();
  SetSwitch(switchindex,0);
 break;
 case 2:
 if(SelectButtonState){RightIndexPos=RightIndexPos+2;}
 else{RightIndexPos=RightIndexPos+15;}
 if (RightIndexPos>=600){RightIndexPos=600;}
  UpdateSetPositions();
  SetSwitch(switchindex,1);
 break;
 default:
 break;
  

}


if (switchindex <=0) {switchindex=0;}
if (switchindex>=SwitchNumbers){switchindex=SwitchNumbers;}
}

void ButtonUp(int MenuLevel){
  Serial.print("UP");
switch (MenuLevel){
  
 case 0:  // top level
 switchindex=switchindex+1;
 break;
 case 1:  // level 1 (left)
 if(SelectButtonState){LeftIndexPos=LeftIndexPos-2;}
 else{LeftIndexPos=LeftIndexPos-15;}
 if (LeftIndexPos<=160){LeftIndexPos=160;}
 UpdateSetPositions();
  SetSwitch(switchindex,0);

 break;
 case 2:
 if(SelectButtonState){RightIndexPos=RightIndexPos-2;}
  else {RightIndexPos=RightIndexPos-15;}
 if (RightIndexPos<=160){RightIndexPos=160;}
 UpdateSetPositions();
  SetSwitch(switchindex,1);
 break;
 default:
 break;
  
}

if (switchindex <=0) {switchindex=0;}
if (switchindex>=SwitchNumbers){switchindex=SwitchNumbers;}
}



void ButtonRight(int MenuLevel){
  uint8_t RocCommand[9];
  
  // nb Looks like you cannot change MenuLevel in a function that called with MenuLevel as a variable, presumably it sets internal variable only?
switch (MenuLevel){
  
 case 0:  // top level
  // get left and right pos from rocrail 
//  MQTT rx topic:[rocnet/ps]  00 00 1C 00 00 05 0F 02 01 08
// MSG  is < NetID:0 Rec:28 Sdr:0 Grp:5 Code[Req]:15 D1=1 D2=8>
RocCommand[0]=0;
RocCommand[1]=0;
RocCommand[2]=SW_bus[switchindex];
RocCommand[3]=0;
RocCommand[4]=0;
RocCommand[5]=0x05; //group
RocCommand[6]=0x0F; //code
RocCommand[7]=0x02; //length
RocCommand[8]=SW_addr[switchindex]; //was 1
RocCommand[9]=SW_addr[switchindex]; //was 8

  MQTTRocnetSend("rocnet/ps",RocCommand);
  delay(50);
  switchstate=0;
  SetSwitch(switchindex,switchstate);// set to left position 
 break;
 case 1:
  UpdateSetPositions();
 switchstate=1;
  SetSwitch(switchindex,switchstate);// set to right position 
 break;
 case2:
  UpdateSetPositions();

 
 break;

 default:
  
 break;
}


}

void ButtonSelect(int MenuLevel){ 
  char MsgTemp[200];
  int cx;

switch (MenuLevel){
  
 case 0:  // top level
  ParseIndex=0;
  MQTTSend("rocrail/service/client","<model cmd=\"swprops\" />");
 
 break;

 case 1:
 /// alternate switch position
 //UpdateSetPositions();
 SetSwitch(switchindex,switchstate);// like MQTTSend("rocrail/service/client","<sw id=\"wood7\" cmd=\"straight\"   />");
 switchstate=!switchstate;
  
 break;
 case 2:
  // alternate switch position 
 // UpdateSetPositions();
  SetSwitch(switchindex,switchstate);// like MQTTSend("rocrail/service/client","<sw id=\"wood7\" cmd=\"turnout\"   />");
  switchstate=!switchstate; 

 break;
 default:
 break;
  
}
}
*/ 
