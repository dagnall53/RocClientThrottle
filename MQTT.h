  #ifndef _MQTT_h
  #define _MQTT_h
  #include "Arduino.h"
// planning to use MQTT for sorting out better library partitioning 

// DEBUG, use with mosquitto_sub -h 127.0.0.1 -i "CMD_Prompt" -t debug -q 0
#define MAXLOCOS 30  // the number we can store (+1 as we use 0) (limited by length of mqtt message!

//void testConnection  (int Number);
void SetWordIn_msg_loc_value(uint8_t* msg, uint8_t firstbyte, int value);
int Count(unsigned int n,char* id, byte* data,unsigned int Length) ;
char* Attrib(unsigned int Nth,char* id, byte* data,unsigned int Length);// to find string id in data and count  (max 65k)
void MQTTFetch (char* topic, byte* payload, unsigned int Length) ;
void MQTTRocnetSend (char* topic, uint8_t * payload);  //replaces rocsend
void MQTTSend (char* topic, char * payload) ;




void  MQTT_Setup(void);
boolean MQTT_Connected(void);
void MQTT_Loop(void);
void DebugMsgSend (char* topic, char* payload) ;
void DebugSprintfMsgSend(int CX);
void PrintTime(String MSG);
void reconnect(void); 


  #endif

