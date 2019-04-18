#include "Arduino.h"
#include "NVSettingInterface.h"

extern String wifiSSID ;
extern String wifiPassword;
extern String NameOfThisThrottle;
extern void OLED_5_line_display(String L1,String L2,String L3,String L4,String L5);
extern void OLED_5_line_display_p(String L1,String L2,String L3,String L4,String L5);
extern int BrokerAddr;
extern uint8_t Volts_Calibration;
    long analog_sum;
extern long ReadADC(int avg);

 //char receivedChars[numChars];   // an array to store the received data
 String SettingsText;  //Revised code that allows me to get the serial data one character at a time..
 boolean newData = false;
 char rx_byte;  // to try serial input
 int SerioLevel; // for the serial interface for updates

//int MSG_content_length(){
//  int Length;
//  int i;
//  bool EOM; 
//  Length =0;i=0;
//  EOM=false;
//  while (!EOM && i<=numChars) {
//       if (receivedChars[i] == 0){EOM=true;Length=i;}
//       i++;
//       }
//  return Length;
//}

extern uint8_t NodeMCUPinD[12];
extern bool SelectPressed();
extern int ADC_IN;

void CheckForSerialInput(){
  String MSGText;
  String MSGText1;
  String MSGText2;
  String MSGText3;
  String MSGText4;
    String TestData;
    long Timestarted;
    bool UpdateInProgress; 
    long FlashTime;
    char CtrlE;
    CtrlE=5;
    // will try to change wifiSSID,wifiPassword
    UpdateInProgress=false;
    if (wifiSSID=="Router Name"){UpdateInProgress=true;Serial.println(" Forcing request for new entries as Default Router name has not been set in Secrets.h");
                                    Serial.println("--Serial port update Started--");
                                    Serial.print("  SSID currently is <");Serial.print(wifiSSID);Serial.print("> Password is <");Serial.print(wifiPassword); Serial.println(">");
                                    Serial.println("Type in New SSID");SerioLevel=1;newData = false; SettingsText = ""; 
                                    OLED_5_line_display("EEPROM settings required","Use Serial port @115200","To enter WiFi details","Press Select"," to start");
                                 }else{
                                    Serial.println("");
                                    Serial.println(F("--- To enter new wifi details / Press Select or type 'xxx' BEFORE wifi connects"));
                                    Serial.println(F("-- Use 'Newline' OR 'CR' to end input line  --"));
                                    Serial.println(F("Starting~~~~~~~~~~~~~~~~~~~~~~~~waiting~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Timeout "));
                                    delay(10);Serial.print(CtrlE);delay(100); OLED_5_line_display_p("","","Pausing for Serial I/O","type 'xxx' to start","or 'Select'");
                                    newData = false; SettingsText = ""; 
                                       }
    Timestarted=millis();
    FlashTime=millis();
    bool LAMP;

    long cal_input;
  // test to see if we can sense if serial cable connected -- Answer no we cannot!! only the Leonardo can  if (Serial){OLED_5_line_display_p("","","","Serial ON","");}else{OLED_5_line_display_p("","","","Serial OFF","");}
  // but we can test the "select" button..  
    while ((millis()<= Timestarted+4000) || UpdateInProgress) {
       ///   toggle  signal lamp
      if ((millis()>= FlashTime)&& (!UpdateInProgress)) { LAMP=!LAMP; FlashTime=millis()+50; digitalWrite (NodeMCUPinD[0] , LAMP) ;Serial.print("~");}
      
      delay(10); // Allow esp to process other events .. seems to be needed,                                   
      recvWithEndMarker();
      if ( (SelectPressed()&& (SerioLevel==0) )||(newData == true) )
             {
                          if (newData == true){
                            //TestData=receivedChars;
                            TestData=SettingsText; // new input method to capture one character at a time?
                            }
                          //Serial.print("Data <");Serial.print(TestData);Serial.println(">"); 
                          //("> Looking for {");Serial.print(LookFor);Serial.println("}");
                          switch (SerioLevel){ 
                          case 0:
                                 if ((TestData=="xxx\0")||(TestData=="XXX\0")|| SelectPressed()){
                                    UpdateInProgress=true;
                                    //display.clear(); display.drawString(64, 32, "Type in New SSID"); display.display();
                                    OLED_5_line_display("Type in New SSID",""," ","","");
                                    Serial.println("-");
                                    Serial.println("--Update EEPROM Started--");
                                    Serial.print("  SSID currently is <");Serial.print(wifiSSID);Serial.println(">"); 
                                    Serial.println("Type in New SSID");newData = false; SettingsText = ""; SerioLevel=1;
                                     }
                          break;
                          case 1: if ( TestData.length()>=2) {wifiSSID=TestData;} newData = false; SettingsText = ""; SerioLevel=2;
                          MSGText1="SSID     <";
                          MSGText1+=wifiSSID;
                          MSGText1+=">";
                                   OLED_5_line_display(MSGText1,"Type Password"," "," "," ");
                                  // display.clear(); display.drawString(64, 12, MSGText); display.display();
                                   Serial.print(" SSID<");Serial.print(wifiSSID);Serial.print("> current Password<");Serial.print(wifiPassword); Serial.println(">");
                                   Serial.println("Type in New Password");
                                    
                          break;

                          case 2:   
                                 if ( TestData.length()>=2) {wifiPassword=TestData;} newData = false; SettingsText = ""; SerioLevel=3;
                                 MSGText2="Password <";
                                 MSGText2+=wifiPassword;
                                 MSGText2+=">";
                                 OLED_5_line_display(MSGText1,MSGText2,"Broker addr?"," "," ");  //display.drawString(64, 24, MSGText); display.display();
                                 Serial.print(" SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                 Serial.print("Broker Addr:");Serial.println(BrokerAddr);Serial.println("Type in MQTT Broker address");
                                    
                          break;
                          case 3:
                                 if ( TestData.length()>=2) {BrokerAddr= TestData.toInt();} newData = false; SettingsText = ""; SerioLevel=4;
                                 MSGText3="Broker Addr<";
                                 MSGText3+=BrokerAddr;
                                 MSGText3+=">";
                                 OLED_5_line_display(MSGText1,MSGText2,MSGText3," This Throttle's Name ?","");
                                 //display.drawString(64, 32, MSGText); display.display();
                                 Serial.print("Broker Addr:");Serial.print(BrokerAddr);Serial.print(" WiFi SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                 Serial.print("Current name <");Serial.print(NameOfThisThrottle);Serial.println(">");
                                 Serial.println("Type Throttle Name");
                          
                          break;
                          case 4:
                                 if ( TestData.length()>=2) {NameOfThisThrottle=TestData;} newData = false; SettingsText = ""; SerioLevel=5;
                                 MSGText4="Name<";
                                 MSGText4+=NameOfThisThrottle;
                                 MSGText4+=">";
                                 OLED_5_line_display(MSGText1,MSGText2,MSGText3,MSGText4," sss to save or rrr to retry");
                                 Serial.print("Summary: Name of Throttle:");Serial.print(NameOfThisThrottle);
                                 Serial.print(" Broker Addr:");Serial.print(BrokerAddr);Serial.print(" WiFi SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                 Serial.println("Type sss to save, rrr to return to start ccc to add cal ");
                          
                          break;
                                
                          case 5:
                                 Serial.print("Summary: Name of Throttle:");Serial.print(NameOfThisThrottle);
                                 Serial.print(" Broker Addr:");Serial.print(BrokerAddr);Serial.print(" WiFi SSID<");Serial.print(wifiSSID);Serial.print("> Password<");Serial.print(wifiPassword); Serial.println(">");
                                  
                           if (TestData=="sss\0"){
                                    //display.clear(); display.drawString(64, 32, "EEPROM Updated"); display.display();
                                    Serial.println("I will now save this data and continue");
                                    WriteWiFiSettings();
                                    UpdateInProgress=false;
                                    newData = false; SettingsText = ""; SerioLevel=6;
                                    break;
                                    }
                           if (TestData=="rrr\0"){ 
                                OLED_5_line_display("Resuming Serial Input","Type in xxx to restart",""," ","");
                                Serial.println("-----------------");Serial.println("---Starting again---");Serial.println(" Type xxx again to re-start sequence");
                                newData = false; SettingsText = ""; SerioLevel=0;
                                 break;}
                                 
                           if (TestData=="ccc\0"){
                               analog_sum=ReadADC(100); //
                               MSGText4="Current V batt <";MSGText4+=analog_sum;MSGText4+=">";  
                                OLED_5_line_display("Starting voltage calibration","Type in actual V as xxx",MSGText4,"","");
                               
                                Serial.print(MSGText4);Serial.println(" Type in actual voltage without decimal (xxx)");
                                newData = false; SettingsText = ""; SerioLevel=8;
                                 break;
                                 }
                           
                          break; 

                          case 6:
                            newData = false; SettingsText = ""; 
                            Serial.println("Wait to try reconnect, or turn off to restart with new values");
                            Serial.println("Type sss to save, rrr to return to start");
                          break;
                          case 8:
                          if ( TestData.length()>=2) {cal_input= TestData.toInt();} 
                          if ((cal_input>=200)&&(cal_input<=600)) { // valid input
                                Serial.print("adjusted Vcal from<");Serial.print(Volts_Calibration);
                                cal_input=(Volts_Calibration*cal_input)/(analog_sum); // calc as long 1% error on wifi starting
                                Volts_Calibration= cal_input;// convert to uint8_t
                                newData = false; SettingsText = ""; 
                                Serial.print("> to <");Serial.print(Volts_Calibration); Serial.println("> ");    }
                                else {Serial.println("- unchanged calibration factor -"); }
                            analog_sum=ReadADC(100); //
                            MSGText4="V Batt <";MSGText4+=analog_sum;MSGText4+=">";  
                            OLED_5_line_display(MSGText4," In batt measurement cal","","","");
                               
                            Serial.println(MSGText4);   
                            Serial.println("Type 'sss' to save this calibration or 'ccc' to check and repeat");newData = false; SettingsText = ""; SerioLevel=9;
                           break;
                           case 9:
                           if (TestData=="sss\0"){
                                    //display.clear(); display.drawString(64, 32, "EEPROM Updated"); display.display();
                                    Serial.println("I will now save this data and continue");
                                    WriteWiFiSettings();
                                    UpdateInProgress=false;
                                    newData = false; SettingsText = ""; SerioLevel=6;
                                    break;
                                    }
                           if (TestData=="rrr\0"){ 
                                OLED_5_line_display("Resuming Serial Input","Type in xxx to restart",""," ","");
                                Serial.println("-----------------");Serial.println("---Starting again---");Serial.println(" Type xxx again to re-start sequence");
                                newData = false; SettingsText = ""; SerioLevel=0;
                                 break;}
                            if (TestData=="ccc\0"){ 
                                  analog_sum=ReadADC(100);
                                  MSGText4=" Measured <";MSGText4+=analog_sum;MSGText4+="";  
                                  OLED_5_line_display("Repeating voltage calibration","Type in measured voltage without decimal xxx ",MSGText4," ","");
                               
                                Serial.print(MSGText4);Serial.println("> Type in actual voltage without decimal (xxx)");
                                newData = false; SettingsText = ""; SerioLevel=8;
                                 break;
                                 }
                                 
                            Serial.println("Please type 'sss' to save, 'ccc' to recheck Batt cal, or 'rrr' to return to start");
                            newData = false; SettingsText = ""; 
                                  
                            
                            Serial.println("Non valid input");
                           break;
                           
                          default:
                            newData = false; SettingsText = ""; 
                            Serial.println("Not Understood");  
                          break; 
                         }
                          
                            }
      showNewData();  // only shows results if newData=true  and not understood (can happen if looping after a rrr)
      } // end of main while serial input loop
}


void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
       while (Serial.available() > 0 ) {
           rc = Serial.read();
           if ((rc != 10)&&(rc != 13) ){ 
                SettingsText+=rc;
                //receivedChars[ndx] = rc;
                Serial.print(rc);delay(1);
                ndx++; 
                if (ndx >= numChars) { ndx = numChars - 1;} //truncate if too big to avoid filling up  
                                       }
                                       else { 
                                        Serial.print("<CR>");delay(1);
                                         newData = true;
                                         //receivedChars[ndx] = '\0';// replace NL/CR with /0 terminator. Mark that new data is available, but do not increment ndx
                                         SettingsText+='\0';
                                             }
                                        }ndx = 0; //once all serial data has been processed, reset the ndx pointer
}

void showNewData() {
    if (newData == true) {
                Serial.println(" ");
                Serial.print("Seen but not understood <");
                Serial.print(SettingsText);
                Serial.println(">    ");
              // to assist debug serial io //Serial.print("Ascii is <"); for (int i=0; i<=(numChars) ;i++) { Serial.print(int(receivedChars[i]));Serial.print("> <");}
                newData = false; SettingsText = ""; 
                }
    }






void WriteWiFiSettings(){
    Serial.println(" --Writing the SSID, Password, BrokerAddr and Throttle_Name to EEPROM-- ");
    writeString(ssidEEPROMLocation,wifiSSID);delay(10);
    writeString(passwordEEPROMLocation,wifiPassword);delay(10);
    writeString(ThrottleNameEEPROMLocation,NameOfThisThrottle);delay(10);
    EEPROM.write(BrokerEEPROMLocation,BrokerAddr);delay(10);
    EEPROM.write(CalEEPROMLocation, Volts_Calibration);
    EEPROM.commit();delay(100);
}

void TestFillEEPROM(int d){
  #ifdef _EEPROMdebug
  Serial.print("~~~WARNING--FILLING EEPROM with int<");Serial.print(d);Serial.println(">~~");
  #endif
    for(int i=0;i<=EEPROM_Size;i++) { EEPROM.write(i,d); }
     EEPROM.commit();delay(100);// 
}

void writeString(int add,String data)
{
  int _size = data.length();
  int i;
#ifdef _EEPROMdebug
       Serial.print("Writing :");Serial.print(_size);Serial.print(" bytes of data <");Serial.print(data);Serial.print("> at EEprom index starting:");Serial.println(int(add));
#endif
  for(i=0;i<_size;i++) { if ((add+i) < (EEPROM_Size)){EEPROM.write(add+i,data[i]);
                                       // Serial.print("Writing<");Serial.print(data[i]);Serial.print(">  to offset:");Serial.print(i);Serial.print("  index:");Serial.println(add+i);
                                                       } // write the data up to the point it would overflow the eeprom
                                       else{ // Serial.print("Stopping overflow Ignoring <");Serial.print(data[i]);Serial.print("> ");
                                           } 
                                       }
                                       //Got all the data, so add the \0
                        if ((add+i) <= (EEPROM_Size-1)){
                                       EEPROM.write(add+i,'\0');
                                       // Serial.print("Adding end_mark     to offset:");Serial.print(i);Serial.print("  index:");Serial.println(add+i);
                                        } 
                                       else{
                                        EEPROM.write(EEPROM_Size,'\0');
                                        //Serial.print("Adding end mark at end EEPRom size index:");Serial.println(EEPROM_Size);
                                        }
                        //Add termination null character for String Data
}
 
 
String read_String(int add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len;
  unsigned char k;
    //     #ifdef _EEPROMdebug
    //     Serial.print("Starting Read EEprom Data at Addr<");Serial.print(int(add));Serial.println("> ");
    //     #endif
  len=0;
  k=EEPROM.read(add);
    while((k != '\0') && (len<99) && ((len+add)<=EEPROM_Size))   //Read until null character, or 100 or end of eeprom
  {     
    k=EEPROM.read(add+len);
    //#ifdef _EEPROMdebug
    //   Serial.print(len);Serial.print("  ");Serial.print(add+len);Serial.print("  ");Serial.println(k);
    //#endif
    data[len]=k; 
    len++;
  }
  data[len]='\0';
  #ifdef _EEPROMdebug
  Serial.print("EEprom data from Addr:");Serial.print(int(add));Serial.print(" is <");Serial.print(data);Serial.println("> ");
  #endif
    return String(data);
}



