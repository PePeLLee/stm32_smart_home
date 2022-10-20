#ifndef RF_RECEIVE_H
#define RF_RECEIVE_H
#include <RCSwitch.h>
//#define DEBUG_RF
static const char* bin2tristate(const char* bin);
static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength);


class RF_receiver{
  enum Type{ 
    TEMP = 1,
    HUM  = 2,
    BAT  = 3,
    BATM = 4    
  };
  STM32* stm;
  byte bt_message[2000];
  int  lastPos = 0;
  RCSwitch mySwitch = RCSwitch();
  int bt_step =0;
  
  public:
  RF_receiver(STM32* stm){
    this->stm = stm;
    Serial1.begin(115200);//PA10 -> TX  HM-10
                          //PA9  -> RX  HM-10
    Serial1.setTimeout(20);
    mySwitch.enableReceive(digitalPinToInterrupt(PB15));
  };  
  void loop(){
    bt_loop();
    rf_loop();
  };
  
  unsigned long long lastRequest = 0;
  void bt_loop();
  void rf_loop();
  int parseDisa(byte* str);
};


#endif
