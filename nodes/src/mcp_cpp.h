#include "board/board.h"
#include "wirish_time.h"
#include "io.h"
#ifndef MCP_CPP_H
#define MCP_CPP_H

McpExtender::McpExtender(STM32* stm, int seq):PinDevice(stm){
  sprintf(devDesc, "Mcp%d", seq);
  broadcastDelay = 700;
  in_out=(seq<4)?OUTPUT:INPUT;
  mcp.begin(seq);
  mcp.setupInterrupts(true,false,LOW);
  // mcp.begin_I2C(0x24);//MCP23XXX_ADDR+seq);
  // while(mcp->begin() != 0){
  //   Serial.println("Initialization of the chip failed, please confirm that the chip connection is correct!");
  //   delay(500);
  // }
  
  this->seq = seq;
  totalItems = 16;
  #ifdef DEBUG_1
    delay(1000);
  #endif
  for(int i=0; i<totalItems; i++){
    pins[i].valSince= 0;
    // mcp->pinMode((DFRobot_MCP23017::ePin_t)i, in_out);
    mcp.pinMode(i, in_out);
    pins[i].lastVal = HIGH;
    if(in_out == OUTPUT){
      mcp.digitalWrite(i, pins[i].lastVal);
    }else{
      mcp.pullUp(i, HIGH);
      mcp.setupInterruptPin(i,CHANGE);
      // DEBUG_1V("Setup interrupt ");DEBUG_1L(i); 
    }
  }
  if(in_out == OUTPUT){
    this->devNo = Can_message::Device::MCP20 + seq;
  }else{      
    this->devNo = Can_message::Device::MCP20 + seq - 1 ;
  } 
  // delay(1000);
};

bool McpExtender::inputAvailable(){
  if(in_out == OUTPUT) return false;
  if(mcp.getLastInterruptPin()!=255){
     DEBUG_1L("Input available");
    return true;
  }
  if((verInputTime != 0) && (verInputTime < millis())){
    DEBUG_1L("Verify input");
    verInputTime = 0;
    return true;
  };

  return false;
};

#endif
