#ifndef MODBUS_H_H
#define MODBUS_H_H
#include <ModbusMaster.h>

#define MAX485_DE     PB15

class Modbus:public Device{  
  protected:
    void broadcastItemState(int);
    // bool isItemValidBcast(int ){};
    byte firstAddrBuffered;
    byte modbusResult;
    ModbusMaster node;
    struct{
      short addr;
      short multiplier;
    } addresses[30];
  public:
    Modbus(STM32* stm);
    // void loop();
    void maintenance(Can_message::message* msg);
    void canCommand(Can_message::message* msg){};
    // void refreshData(){};
    float toFloat(int);
};
#endif
