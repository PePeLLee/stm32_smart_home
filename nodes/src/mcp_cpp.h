#include "board/board.h"
#include "wirish_time.h"
#include "io.h"
#ifndef MCP_CPP_H
#define MCP_CPP_H

McpExtender::McpExtender(STM32* stm, int seq)
  : PinDevice(stm) {
  sprintf(devDesc, "Mcp%d", seq);
  broadcastDelay = 700;
  in_out = (seq < 4) ? OUTPUT : INPUT;
  // mcp.begin(seq);
  mcp = new MCP23017(0x20 + seq);
  Wire.begin();
  mcp->init();

  mcp->portMode(MCP23017Port::A, (in_out == OUTPUT) ? 0 : 255);
  mcp->portMode(MCP23017Port::B, (in_out == OUTPUT) ? 0 : 255);

  mcp->writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A
  mcp->writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B

  mcp->writeRegister(MCP23017Register::IPOL_A, 0x00);
  mcp->writeRegister(MCP23017Register::IPOL_B, 0x00);
  // mcp.setupInterrupts(true,false,LOW);
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
  bitsA = 0xFF;
  bitsB = 0xFF;
  if (in_out == OUTPUT) {
    mcp->writePort(MCP23017Port::A, bitsA);
    mcp->writePort(MCP23017Port::B, bitsB);
  }
  for (int i = 0; i < totalItems; i++) {
    pins[i].valSince = 0;
    pins[i].lastVal = HIGH;
  }
  if (in_out == OUTPUT) {
    this->devNo = Can_message::Device::MCP20 + seq;
  } else {
    this->devNo = Can_message::Device::MCP20 + seq - 1;
  }
  // delay(1000);
};

bool McpExtender::inputAvailable() {
  if (in_out == OUTPUT) return false;
  if (allButtons != oldButtons) {
    oldButtons = allButtons;
    DEBUG_5L("Sate changed");
    return true;
  } else
    return false;

  return false;
};

#endif