// CAN Receive Example
//
// #define DEBUG_1
// #define DEBUG_2
// #define DEBUG_3
#define DEBUG_4
//#define MCP1
//#define MCP2
#define DEV_ID 24
// #define DEBUG_1
//#define MCP20_OUT
//MCP21_OUT
//#define MCP24_IN
// #define PCA1
// #define PCA2
//#define COLOR_LED
#define DS18
#define MODBUS

#include <mcp2515_can.h>
#include <SPI.h>
#include "bluepill_common.h"

                      // Array to store serial string

mcp2515_can CAN(PA4);                               // Set CS to pin 10



STM32 device;
void setup()
{
  device.setup(&CAN);                           // Configuring pin for /INT input
  pinMode(PC13, OUTPUT);
 
}
long long unsigned lastTime=0;
void loop()
{
//  Serial.println(millis() - lastTime);
//  lastTime = millis();
  device.loop();

}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
