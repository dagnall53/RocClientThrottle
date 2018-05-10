// extern variables for display and selection stuff
extern int MenuLevel;
extern int locoindex;
extern int fnindex;
extern int speedindex;
extern bool directionindex;
extern int LocoNumbers;
//extern int y;
extern uint32_t ButtonPressTimer;
extern uint32_t LcPropsEnabled;
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

void SetLoco(int locoindex,int speedindex){
  bool Dir;
   char MsgTemp[200];
  int cx;
  int SpeedSelected;
  SpeedSelected=0;
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

#ifdef Rotary
SpeedSelected=speedindex;
#endif
// if (speedindex>=0){cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" fn=\"false\" dir=\"true\"  throttleid=\"RocClientThrottle\" />",Str2Chr(LOCO_id[locoindex]),SpeedSelected);}
 //     else{cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" fn=\"false\" dir=\"false\"  throttleid=\"RocClientThrottle\" />",Str2Chr(LOCO_id[locoindex]),SpeedSelected);}

Dir=LastDir;
if (speedindex>=1){Dir=true;}
if (speedindex<=-1){Dir=false;}  // this set of code tries to ensure that when speed = 0 it uses the last direction set. 

 if (Dir) {LastDir=true;cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" dir=\"true\"  throttleid=\"RocClientThrottle\" />",Str2Chr(LOCO_id[locoindex]),SpeedSelected);}
      else{LastDir=false;cx=sprintf(MsgTemp,"<lc id=\"%s\"  V=\"%d\" dir=\"false\"  throttleid=\"RocClientThrottle\" />",Str2Chr(LOCO_id[locoindex]),SpeedSelected);}

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
   // SpeedSelected="STOP";
   // Serial.print(SpeedSelected);

   
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


#endif

#ifdef Rotary
SpeedSelected=ThrottlePosition;
display.drawString(25,28,"Speed:");
#endif

display.drawString(90,28,SpeedSelected);


    
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
  Serial.print("DOWN");
switch (MenuLevel){
  
 case 0:  // top level
 locoindex=locoindex-1;
 break;
 case 1:  // top level
 speedindex=speedindex+1; // works better to increment with this button
 #ifndef Rotary
   SetLoco(locoindex,speedindex);// rotary sets the loco in a different place
 #endif
 break;
 case 2:
 fnindex=fnindex+1;
 if (fnindex>=8){fnindex=8;}
 break;
 default:
 break;
  

}
if (speedindex>=4){speedindex=4;}

if (locoindex <=0) {locoindex=0;}
if (locoindex>=LocoNumbers){locoindex=LocoNumbers;}
}

void ButtonUp(int MenuLevel){
  Serial.print("UP");
switch (MenuLevel){
  
 case 0:  // top level
 locoindex=locoindex+1;
 break;
 case 1:  // level 1
 speedindex=speedindex-1;
  #ifndef Rotary
     SetLoco(locoindex,speedindex);// rotary sets the loco in a different place
 #endif
 break;
 case 2:
 fnindex=fnindex-1;
 if (fnindex<=0){fnindex=0;}
 break;
 default:
 break;
  
}
if (speedindex<=-4){speedindex=-4;}
if (locoindex <=0) {locoindex=0;}
if (locoindex>=LocoNumbers){locoindex=LocoNumbers;}
}


void ButtonRight(int MenuLevel){
  // nb Looks like you cannot change MenuLevel in a function that called with MenuLevel as a variable, presumably it sets internal variable only?
switch (MenuLevel){
  
 case 0:  // top level
      #ifdef Rotary
       ThrottlePosition=0; ThumbWheel.write(0); // set Throttle pos to zero  

      #endif
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
  LcPropsEnabled=millis()+1500; // allow 1.5 sec for inital reading of props list: After this the Mqtt parse is disabled. 
  ParseIndex=0;
  AllDataRead=false;
  MQTTSend("rocrail/service/client","<model cmd=\"lcprops\" />");
  
 }
 else {     
       SoundLoco(locoindex,2);
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
      if (ThrottlePosition==0){ SoundLoco(locoindex,2); }//toot fn 2 (loco index) 
       else{ ThrottlePosition=0; ThumbWheel.write(0); SetLoco(locoindex,ThrottlePosition);}

#endif
   
 break;
 case 2:
  SoundLoco(locoindex,fnindex);
  
break;
 default:
 break;
  
}
}

