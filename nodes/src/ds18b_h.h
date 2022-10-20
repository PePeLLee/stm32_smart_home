#ifndef DS18B_H_H
#define DS18B_H_H

#include <DallasTemperature.h>
#define WIRE_PIN PB3
#define TEMPERATURE_PRECISION 9

class Ds18b:public Device{
  protected:
    void broadcastItemState(int);
    void refreshData();
    OneWire* oneWire;
    DallasTemperature* sensors;
    int adrNo = 0;
    char buff[20];  

    struct single_meter{
      int lastVal = 0;
      DeviceAddress addr;
    };
    single_meter  meters[10];
    int getAddr(int);
    char* addrToStr(DeviceAddress addr);
    int getTemp(int);
	public:
    Ds18b(STM32* stm);
    // void loop();
    void maintenance(Can_message::message* msg);
    void canCommand(Can_message::message* msg){};
};
#endif
