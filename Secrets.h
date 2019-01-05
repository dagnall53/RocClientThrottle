 #define SSID_RR "Router Name"
       #define PASS_RR "Password for Router"
       int BrokerAddrDefault = 11; // sub ip of your MQTT broker (NOT 0 or 255); 
       #define ThrottleNameDefault "DagThrottle"  // change this to Identify different throttles to Rocrail
 // ESP32 compiler differences should be defined by the board selection in the compiler 
// hardware
// This is Board dependant

  // These pin definitions are same for both ESP8266 and ESP32.
  static const uint8_t OLED_SCL = 4;  //also known as D2 on esp 8266, but
  static const uint8_t OLED_SDA = 5;  // also known as D1 on esp 8266
// The following hardware pinouts differ for ESP32 and ESP8266 variants
    #ifdef ESP8266
    // notes  ESP12 on oled battery board /// D8 has somethng dragging its output down  in pull up to 0.8v
    // Available A0 D3 D4 D8 D9 D10
     //D1 and D2 are oled scl/sda
     const int EncoderPinA = D9;     // == "D9"  is  the designation of the A Rotary_Switch pushbutton pin (common is connected to ground //change in v9
     const int EncoderPinB = D4;     // == "D4"   is the designation of the B Rotary_Switch pushbutton pin (common is connected to ground
     const int RightButton = D3;     // the designation of the right pushbutton pin on wemos 18650 oled board
     const int SelectButton = D5;     // the designation of the select pushbutton pin
     const int UpButton = D6;     // the designation of the up pushbutton pin
     const int DownButton = D7;     // the designation o0 the down pushbutton pin
     const int LeftButton = D8  ;    // left NOT used yet 
    #endif

    #ifdef ESP32  // on esp pins are strictly Px = pin x not Dx = somethong strange as on the ESP8266 
     // ?? GPIO06 through GPIO11 are reserved for the FLASH. You cannot use them .https://github.com/espressif/arduino-esp32/issues/1411
     // GPIOs 34-39 are input-only no pullups1
     // OLED_SCL = pin P4;
     // OLED_SDA = pin P5;
     //All pins connect between Ground and designated pin
    
     // const int LED_BUILTIN =16;       // already defined .. onboard LED is pin GPIO 16 ?
     const int EncoderPinA = 25;     // P34  is  the designation of the A Rotary_Switch pushbutton pin (common is connected to ground
     const int EncoderPinB = 26;     // P35 is the designation of the B Rotary_Switch pushbutton pin (common is connected to ground
     const int SelectButton = 27;     // P33 is the number of the select pushbutton pin
 
     const int RightButton = 19;     // P32 is the designation of the right pushbutton pin on wemos 18650 oled board
     const int UpButton = 18;     // P25 is the number of the up pushbutton pin
     const int DownButton =17;     // P26 is the number of the down pushbutton pin
     const int LeftButton = 16;    // P27 is the number of the left pushbutton pin NOT used yet 
    #endif





      
/* PIN References... Also defined somewhere else in the esp8266 included code so do not unhide this section!!!...
  static const uint8_t D0   = 16;  and Red Led on NodeMcu V2 (not present on NodeMCU v3)
  static const uint8_t D1   = 5;
  static const uint8_t D2   = 4;
  static const uint8_t D3   = 0;
  static const uint8_t D4   = 2;  and Blue Led on ESP8266 also is i2c clk?
  static const uint8_t D5   = 14;
  static const uint8_t D6   = 12;
  static const uint8_t D7   = 13;
  static const uint8_t D8   = 15;
  static const uint8_t D9   = 3;  ?? is this RXD0
  static const uint8_t D10  = 1;  ?? is this TXD0


  #define NodeMCUPinD[SignalLed] 2 // same as PIN D4!

*/
