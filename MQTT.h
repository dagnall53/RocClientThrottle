  #ifndef _MQTT_h
  #define _MQTT_h
  #include "Arduino.h"
// planning to use MQTT for sorting out better library partitioning 

// FOR DEBUG and to monitor connections, use with mosquitto_sub -h 127.0.0.1 -i "CMD_Prompt" -t debug -q 0

// How many locos and functions can we use...
  #define MAXLOCOS 126  // the number we can store (+1 as we use 0) (limited by memory size 
                      // >150 crashes the throttle, 100 seems a practical max ! The exact limit is unexplored 
  #define N_Functions 15   // F0 to F(N_Functions-1) ie if '15', you can get to 'F14'

//void testConnection  (int Number);
void SetWordIn_msg_loc_value(uint8_t* msg, uint8_t firstbyte, int value);
int Count(unsigned int n,char* id, byte* data,unsigned int Length) ;
char* ParseforAttribute(String First, String id, char End, byte* data,unsigned int Length); // to find string First then attribute {string id}ending in End in data 

char* Attrib(unsigned int Nth,String id, byte* data,unsigned int Length);// to find Nth string id in data and count  (max 65k)

void ParsePropsLocoList(byte loco, byte* payload, unsigned int Length);
void ParseSwitchList(byte Switch, byte* payload, unsigned int Length);
char*  Show_ROC_MSG(uint8_t Message_Length);
void SetWordIn_msg_loc_value(uint8_t* msg, uint8_t firstbyte, int value) ;
int getTwoBytesFromMessageHL( uint8_t* msg, uint8_t highloc);
int IntFromPacket_at_Addr(uint8_t* msg, uint8_t highbyte);
void dump_byte_array(byte* buffer, byte bufferSize);
void MQTTFetch (char* topic, byte* payload, unsigned int Length) ;
void MQTTRocnetSend (char* topic, uint8_t * payload);  //replaces rocsend
void MQTTSend (String topic, String payload) ;




void  MQTT_Setup(void);
boolean MQTT_Connected(void);
void MQTT_Loop(void);
void DebugMsgSend (String topic, String payload) ;
void DebugSprintfMsgSend(int CX);
void PrintTime(String MSG);
void MQTT_ReConnect(void); 


  #endif

