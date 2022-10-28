#include "io.h"
#ifndef MCP_H_H
#define MCP_H_H

#ifdef DEBUG_5 
  #define DEBUG_5V(x) Serial.print(x);
  #define DEBUG_5L(x) Serial.println(x); delay(10);
#else
  #define DEBUG_5V(x)
  #define DEBUG_5L(x)
#endif

// #include <DFRobot_MCP23017.h>
#include "Adafruit_MCP23017.h"

// class McpPin: public Pin{
// 	Adafruit_MCP23017* mcp;	
// 	public:
// 	void changeValue(byte val);
// 	void checkInput();
	
	
// 	McpPin(int i, int in_out, STM32* stm, Adafruit_MCP23017* mcp, int seq):Pin(stm){

// 		mcp->pinMode(i, in_out);
//     this->lastVal = HIGH;
    
//     this->in_out = in_out;
// 		if(in_out == OUTPUT){
// //      this->lastVal = HIGH;
//       mcp->digitalWrite(i, this->lastVal);
//       this->devNo = Can_message::Device::MCP20 + seq;
//       sprintf(devDesc, "MCP Out");
//     }else{      
//       mcp->pullUp(i, HIGH);
//       sprintf(devDesc, "MCP In");
//       this->devNo = Can_message::Device::MCP20 + seq - 1 ;
// 		} 
// 		this->gpio_no = i;
// 		this->mcp = mcp;
// 	};
// };

class McpExtender:public PinDevice{
  protected:
    // DFRobot_MCP23017 *mcp;//(Wire, /*addr =*/0x27);
		Adafruit_MCP23017 mcp;
		int seq;
    int in_out;
	  // int currRead;
		// McpPin* pins[16];
    unsigned long long verInputTime = 0;
    struct single_pin{
      short lastVal;
      unsigned long long valSince;
    };
    single_pin  pins[16];
    bool isItemValidInput(int){return in_out != OUTPUT;};
    bool isItemValidCommand(int itm){ return in_out == OUTPUT;};
    inline bool isDimmable(int itm){return false;};
    inline int getVal(int itm){ return pins[itm].lastVal; };
    inline int setVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ pins[itm].lastVal = (val==0)?LOW:HIGH; return pins[itm].lastVal; };
    inline int getHWVal(int itm){ return mcp.digitalRead(itm); };
    // int getHWVal(int itm){ 
    //   DEBUG_1L("Pre digital read");
    //   byte a = mcp.digitalRead(itm);
    //   DEBUG_1L("Post digital read");
    //   return a; };
    void setHWVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ mcp.digitalWrite(itm, (val==0)?LOW:HIGH); };
    inline unsigned long long getValSince(int itm){ return pins[itm].valSince; };
    inline void setValSince(int itm){ pins[itm].valSince = millis(); verInputTime = pins[itm].valSince + 50;};
    inline void clearValSince(int itm){ pins[itm].valSince = 0; };
    bool inputAvailable();

	public:
		McpExtender(STM32* stm, int seq = 0);
    // void loop();
    // void maintenance(Can_message::message* msg);
    // void canCommand(Can_message::message* msg);
    // void checkInput(int gpio_no);
};

#endif
