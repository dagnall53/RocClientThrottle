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
extern bool EncoderMoved;

bool LastDir;
int LastLoco;
void SetLoco(int locoindex,int speedindex){
  bool Dir;
   char MsgTemp[200];
  int cx;
  int SpeedSelected;
  SpeedSelected=speedindex;
// new for rotary encoder, take out the speed steps
#ifndef Rotary 
switch (abs(speedindex)){
  case 0:
  SpeedSelected=0;
  break;
  case 1:
  SpeedSelected=LOCO_V_min[locoindex].toInt();
    break;
  case 2:
  SpeedSelected=LOCO_V_mid[locoindex].toInt();
  break;
  case 3:
  SpeedSelected=LOCO_V_cru[locoindex].toInt();
    break;
  case 4:
  SpeedSelected=LOCO_V_max[locoindex].toInt();
    break;
  case 5:
  SpeedSelected=LOCO_V_max[locoindex].toInt();
     break;
} 
#endif


Dir=LastDir;
LastLoco=locoindex;
if (speedindex>=1){Dir=true;}
if (speedindex<=-1){Dir=false;}  // this set of code tries to ensure that when speed = 0 it uses the last direction set. 

 if (Dir) {LastDir=true; cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" dir=\"true\"  throttleid=\"%s\" />",Str2Chr(LOCO_id[locoindex]),SpeedSelected,ThrottleName);}
      else{LastDir=false;cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" dir=\"false\"  throttleid=\"%s\" />",Str2Chr(LOCO_id[locoindex]),abs(SpeedSelected),ThrottleName);}

  //Serial.print(LOCO_id[locoindex]);
  Serial.print(" <");Serial.print(MsgTemp);Serial.println(">");
  MQTTSend("rocrail/service/client",MsgTemp);
}
bool LightsState;

void LocoLights(int locoindex,bool state){
  char MsgTemp[200];
  int cx;
  if (state){
  cx=sprintf(MsgTemp,"<lc id=\"%s\"  fn=\"true\"   />",Str2Chr(LOCO_id[locoindex]));}
  else {cx=sprintf(MsgTemp,"<lc id=\"%s\"  fn=\"false\"   />",Str2Chr(LOCO_id[locoindex]));}
  Serial.print(LOCO_id[locoindex]);Serial.println(" Lights setting<");Serial.print(MsgTemp);Serial.println(">");
  MQTTSend("rocrail/service/client",MsgTemp);
  
}

void SetFn(int locoindex,int fnindex, bool state){
  char MsgTemp[200];
  int cx; 
  if (state){  cx=sprintf(MsgTemp,"<fn id=\"%s\" f%d=\"true\"  />",Str2Chr(LOCO_id[locoindex]),fnindex);}
  else { cx=sprintf(MsgTemp,"<fn id=\"%s\" f%d=\"false\"  />",Str2Chr(LOCO_id[locoindex]),fnindex);}
  Serial.print(LOCO_id[locoindex]);Serial.println(" fn set<");Serial.print(MsgTemp);Serial.println(">");
  MQTTSend("rocrail/service/client",MsgTemp);
}

void SoundLoco(int locoindex,int fnindex){
  if (fnindex==0){LightsState=!LightsState;LocoLights(locoindex,LightsState);}
  else{
      SetFn(locoindex,fnindex,true);
      delay(10);
      SetFn(locoindex,fnindex,false);
      }
}

extern void Picture();

void DoDisplay(int MenuLevel){
     //display.clear;
String SpeedIndexString = String(speedindex);
String FnIndexString= String(fnindex);
String SpeedSelected;   
String TopMessage = "Available Locos:";
TopMessage += (LocoNumbers);
display.setFont(ArialMT_Plain_10);
//display.drawProgressBar(0, 58,127,6, ((y*100)/64));
if (LocoNumbers>=1){
display.drawString(64,54,TopMessage);}
switch (MenuLevel){
  
 case 0:  // top level
    display.setFont(ArialMT_Plain_10);
    display.drawString(64,1,"--- Select Loco ---");
if (LocoNumbers<=0){
  Picture();
    display.setFont(ArialMT_Plain_10);
    display.drawString(64,25," Press to ");
    display.drawString(64,38,"Refresh Loco List");
    display.setFont(ArialMT_Plain_16);
    
}
  else{  
    if (locoindex>=1){
     display.setFont(ArialMT_Plain_10);
     display.drawString(64,16,LOCO_id[locoindex-1]);
        }
     if (locoindex<=LocoNumbers-1){
     display.setFont(ArialMT_Plain_10);
     display.drawString(64,40,LOCO_id[locoindex+1]);
        }    
     display.setTextAlignment(TEXT_ALIGN_CENTER);
     display.setFont(ArialMT_Plain_16);
    display.drawString(64,26,LOCO_id[locoindex]);
   
  }
      break;
 case 1: // selected loco, set speed
 //show loco
 if (LocoNumbers<=0){
   display.setFont(ArialMT_Plain_16);
   display.drawString(64,1,"No Loco");
  display.drawString(64,16,"selected");}
   else
    {
     display.setTextAlignment(TEXT_ALIGN_CENTER);
     display.setFont(ArialMT_Plain_16);
     display.drawString(64,1,LOCO_id[locoindex]);
    }
 //show speed 
 SpeedSelected="";
 #ifndef Rotary
    display.drawString(25,28,"Speed:");
    display.drawString(60,28,SpeedIndexString);
    switch (abs(speedindex)){
        case 0:
        SpeedSelected="STOP";
        break;
        case 1:
        SpeedSelected=LOCO_V_min[locoindex];
        break;
        case 2:
        SpeedSelected=LOCO_V_mid[locoindex];
        break;
        case 3:
        SpeedSelected=LOCO_V_cru[locoindex];
        break;
        case 4:
        SpeedSelected=LOCO_V_max[locoindex]; 
        break;
        case 5:
        SpeedSelected=LOCO_V_max[locoindex];
        break;
      }
      display.drawString(90,28,SpeedSelected);

#endif

#ifdef Rotary
//SpeedSelected=String(ThrottlePosition);
display.drawString(25,28," Speed:");
display.drawString(90,28,SpeedIndexString);
#endif




    
      break;
      
case 2: // selected loco, set fn
 //show loco
 if (LocoNumbers<=0){
   display.setFont(ArialMT_Plain_16);
   display.drawString(64,1,"No Loco");
  display.drawString(64,16,"selected");}
   else
    {
     display.setTextAlignment(TEXT_ALIGN_CENTER);
     display.setFont(ArialMT_Plain_16);
     display.drawString(64,1,LOCO_id[locoindex]);
    }
 //show fn
    display.setFont(ArialMT_Plain_10);
    display.drawString(64,20,"press for"); 
    if (fnindex==0){
      display.setFont(ArialMT_Plain_16);
      if (LightsState){display.drawString(64,34,"Lights OFF"); }
      else{display.drawString(64,34,"Lights ON");}
                   }
    else{
    display.setFont(ArialMT_Plain_16);
    display.drawString(25,34," Fn :");
    display.drawString(64,34,FnIndexString);}




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
 #ifdef Rotary
 if (!EncoderMoved){ speedindex=speedindex+9;}
 #endif
 SetLoco(locoindex,speedindex);// rotary sets the loco in a different place
 
 break;
 case 2:
 fnindex=fnindex+1;
 if (fnindex>=8){fnindex=8;}
 break;
 default:
 break;
  

}
#ifndef Rotary
if (speedindex>=4){speedindex=4;}
#endif
#ifdef Rotary
if (speedindex>=100){speedindex=100;}
#endif
if (locoindex <=0) {locoindex=0;}
if (locoindex>=LocoNumbers){locoindex=LocoNumbers;}
}

void ButtonUp(int MenuLevel){
  //Serial.print("UP");
switch (MenuLevel){
  
 case 0:  // top level
 locoindex=locoindex+1;
 break;
 case 1:  // level 1
 speedindex=speedindex-1;
 #ifdef Rotary
 if (!EncoderMoved){ speedindex=speedindex-9;}
 #endif
 SetLoco(locoindex,speedindex);// rotary sets the loco in a different place
  break;
 case 2:
 fnindex=fnindex-1;
 if (fnindex<=0){fnindex=0;}
 break;
 default:
 break;
  
}
#ifndef Rotary
if (speedindex<=-4){speedindex=-4;}
#endif
#ifdef Rotary
if (speedindex<=-100){speedindex=-100;}
#endif
if (locoindex <=0) {locoindex=0;}
if (locoindex>=LocoNumbers){locoindex=LocoNumbers;}
}


void ButtonRight(int MenuLevel){
  // nb Looks like you cannot change MenuLevel in a function that called with MenuLevel as a variable, presumably it sets internal variable only?
//debug
//Serial.print("DEBUG Right button MenuLevel<"); Serial.print(MenuLevel);
//Serial.print("> locoindex is<");Serial.print(locoindex);
//Serial.print("> LastLoco is<");Serial.println(LastLoco);
// adding bit to send lcprops same as select button

if (LocoNumbers<=0){
  Serial.print("sending Loco info request   ");
  Serial.println("<model cmd=\"lcprops\" />");
  ParseIndex=0;
  AllDataRead=false;
  MQTTSend("rocrail/service/client","<model cmd=\"lcprops\" />");
  }
  
switch (MenuLevel){
 case 0:  // top level 
 if (LastLoco!=locoindex){
  //Serial.print("new loco so start at 0 speed");
      speedindex=0;
      #ifdef Rotary
       ThrottlePosition=0; ThumbWheel.write(0);
       // set Throttle pos to zero  
 
      #endif
      }
      else{
   //     Serial.print("Same loco");
      }
 break;
 case 1:  // level 
 /*if (LastLoco!=locoindex);{Serial.print("new loco case 2 so reset throttle");
      speedindex=0;
      #ifdef Rotary
       ThrottlePosition=0; ThumbWheel.write(0);
       // set Throttle pos to zero  

      #endif
 }
 */
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
 if (LocoNumbers<=0){   
 Serial.print("sending Loco info request   ");
 Serial.println("<model cmd=\"lcprops\" />");
 // LcPropsEnabled=millis()+1500; // allow 1.5 sec for inital reading of props list: After this the Mqtt parse is disabled. 
  ParseIndex=0;
  AllDataRead=false;
  MQTTSend("rocrail/service/client","<model cmd=\"lcprops\" />");
  //delay(500);MQTTSend("rocrail/service/client","<model cmd=\"lcprops\" />"); // send twice to try to trigger repeat find of first loco and stop further parsing? //not needed with v004
   }
 else { SoundLoco(locoindex,2);
       
      }
 break;

 case 1:
 #ifndef Rotary
 if (speedindex==0){ //toot fn (index) 
     SoundLoco(locoindex,2);
 }
 else{
   speedindex=0;
   SetLoco(locoindex,speedindex);}
#endif
    #ifdef Rotary
      if (speedindex==0){ SoundLoco(locoindex,2); }//toot fn 2 (loco index) 
       else{  speedindex=0;ThrottlePosition=0;LastThrottlePosition=0; ThumbWheel.write(0); SetLoco(locoindex,speedindex);}

#endif
   
 break;
 case 2:
  SoundLoco(locoindex,fnindex);
  
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
String TopMessage = "Available Turnouts:";
TopMessage += (SwitchNumbers);
display.setFont(ArialMT_Plain_10);
//display.drawProgressBar(0, 58,127,6, ((y*100)/64)); 
display.setTextAlignment(TEXT_ALIGN_CENTER);
if (SwitchNumbers>=1){
display.drawString(64,54,TopMessage);}

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

extern bool buttonState5;
void ButtonDown(int MenuLevel){
  Serial.print("DOWN");
switch (MenuLevel){
  
 case 0:  // top level
 switchindex=switchindex-1;
 break;
 case 1:  // top level
 if(buttonState5){ LeftIndexPos=LeftIndexPos+2;}
 else  {LeftIndexPos=LeftIndexPos+15;}
 if (LeftIndexPos>=600){LeftIndexPos=600;}
  UpdateSetPositions();
  SetSwitch(switchindex,0);
 break;
 case 2:
 if(buttonState5){RightIndexPos=RightIndexPos+2;}
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
 if(buttonState5){LeftIndexPos=LeftIndexPos-2;}
 else{LeftIndexPos=LeftIndexPos-15;}
 if (LeftIndexPos<=160){LeftIndexPos=160;}
 UpdateSetPositions();
  SetSwitch(switchindex,0);

 break;
 case 2:
 if(buttonState5){RightIndexPos=RightIndexPos-2;}
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
