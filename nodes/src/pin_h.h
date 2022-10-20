#include <stdint.h>
#ifndef PIN_H_H
#define PIN_H_H



#define UNDEF 4
#define STS_RESERVED 255
#define STS_UNINIT 254

  byte availPins[] = {PB12, PB13, PB14, PA8, PA15, 
#if !defined(DS18)
  PB3, 
#endif
  PB4, PB5,
#if !defined(RF_RECEIVE) && !defined(MODBUS)
    PA9, PA10, PB15,
#endif
#ifndef I2C
    PB6, PB7, 
#endif
    PB8,  PB9,   PB11, PB10, PB1,  PB0,
#ifndef DS18
    PA3, 
#endif
    PA2, PA1, PA0, PC15, PC14};

class GPIO_Pins:public Device{
  protected:

    struct single_pin{
      // byte  gpio_no = 0;
      byte  reserved:1,
            initialized:1,
            in_out:2;
      short lastVal;
      unsigned long long valSince;
    };
    single_pin  pins[PC15+1];
    
    void broadcastItemState(int);
    bool isItemValidBcast(int );
  public:
    GPIO_Pins(STM32* stm);
    void loop();
    void maintenance(Can_message::message* msg);
    void canCommand(Can_message::message* msg);
    void checkInput(int gpio_no);
    
};
#endif
