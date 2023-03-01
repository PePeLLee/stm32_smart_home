#ifndef PCF_H_H
#define PCF_H_H

#ifdef DEBUG_7 
  #define DEBUG_7V(x) Serial.print(x);
  #define DEBUG_7L(...) Serial.println(__VA_ARGS__); delay(10);
#else
  #define DEBUG_7V(x)
  #define DEBUG_7L(...)
#endif
#define PCF8575_LOW_MEMORY
#include "PCF8575.h"

class PcfExtender:public PinDevice{
  protected:
    // DFRobot_MCP23017 *mcp;//(Wire, /*addr =*/0x27);
		// Adafruit_MCP23017 mcp;
    PCF8575* pcf8575;//(0x20)
		int seq;
    // int in_out;
	  // int currRead;
		// McpPin* pins[16];
    unsigned long long verInputTime = 0;
    uint16_t allButtons, oldButtons;
    struct single_pin{
      short lastVal;
      unsigned long long valSince;
    };
    single_pin  pins[16];
    bool isItemValidInput(int){return true;};
    bool isItemValidCommand(int itm){ return false;};
    inline bool isDimmable(int itm){return false;};
    inline int getVal(int itm){    //DEBUG_7V("Push check time ");DEBUG_7V(itm);DEBUG_7V(" Old Val: ");DEBUG_7V(pins[itm].lastVal); 
      return pins[itm].lastVal; };
    inline int setVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ pins[itm].lastVal = (val==0)?LOW:HIGH; return pins[itm].lastVal; };
    inline int getHWVal(int itm){ return (allButtons&(1UL<<itm))!=0; };
    void setHWVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ /*mcp.digitalWrite(itm, (val==0)?LOW:HIGH);*/ };
    inline unsigned long long getValSince(int itm){ return pins[itm].valSince; };
    inline void setValSince(int itm){ pins[itm].valSince = millis(); verInputTime = pins[itm].valSince + 50;};
    inline void clearValSince(int itm){ pins[itm].valSince = 0; };
    bool inputAvailable(){
      if((verInputTime != 0) && (verInputTime < millis())){
        DEBUG_7L("Verify input");
        verInputTime = 0;
        return true;
      };
      if(allButtons!=oldButtons){
        oldButtons = allButtons; 
        DEBUG_7L("Sate changed"); 
        return true; 
      } else 
        return false; 
    };
    void refreshData(){ allButtons = pcf8575->digitalReadAll(); };

	public:
		PcfExtender(STM32* stm, int seq = 0):PinDevice(stm){
      pcf8575 = new PCF8575(0x24+seq);
      sprintf(devDesc, "PCF%d", seq);
      broadcastDelay = 700;
      refreshDelay = 1;
      pcf8575->begin();     
      this->seq = seq;
      totalItems = 16;
      flips = new short[totalItems];
      #ifdef DEBUG_7
        delay(1000);
      #endif
      for(int i=0; i<totalItems; i++){
        pins[i].valSince= 0;
        pins[i].lastVal = HIGH;
        pcf8575->pinMode(i, OUTPUT);
        pcf8575->digitalWrite(i, HIGH);
      }
      delay(40);
      for(int i=0;i<16;i++){
        pcf8575->pinMode(i, INPUT);
      }
      this->devNo = Can_message::Device::PCF24 + seq;
       
    };
    // void loop();
    // void maintenance(Can_message::message* msg);
    // void canCommand(Can_message::message* msg);
    // void checkInput(int gpio_no);
};

#endif
