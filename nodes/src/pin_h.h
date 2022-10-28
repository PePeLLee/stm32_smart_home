#include "io.h"
#include <stdint.h>
#ifndef PIN_H_H
#define PIN_H_H

#ifdef DEBUG_1 
  #define DEBUG_1V(x) Serial.print(x);
  #define DEBUG_1L(x) Serial.println(x); delay(10);
#else
  #define DEBUG_1V(x)
  #define DEBUG_1L(x)
#endif


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


class PinDevice: public Device{
  protected:
    struct{
      byte pin;
      long long dimmSince;
      short destVal;
      short initVal;
    }dimms[6];
    byte dimmCnt = 0;
    virtual inline short pwmFull(){ return 255; };
    virtual inline void broadcastItemState(int);
    // virtual bool isItemValidBcast(int );
    virtual bool isItemValidInput(int) {return false;};
    virtual bool isItemValidCommand(int itm){return isItemValidBcast(itm);};
    virtual int getVal(int itm) = 0;
    virtual int getHWVal(int itm) = 0;
    virtual int setVal(int itm, int val, byte valType = Can_message::ValType::T_INT) = 0;
    virtual void setHWVal(int itm, int val, byte valType = Can_message::ValType::T_INT) = 0;
    virtual unsigned long long getValSince(int itm)=0;
    virtual void setValSince(int itm)=0;
    virtual void clearValSince(int itm)=0;
    virtual bool inputAvailable()=0;
    virtual inline bool isDimmable(int itm){return true;};
    virtual void setDimmer(byte, short, short);
    virtual void handleDimmers();
  public:
    PinDevice(STM32* stm):Device(stm){};
    void loop(){
      if(inputAvailable()){
        for ( int i=0; i<this->totalItems; i++)
          if(isItemValidInput(i))
            checkInput(i);
      }
      handleDimmers();
      Device::loop();
    };
    virtual void maintenance(Can_message::message* msg){};
    void canCommand(Can_message::message* msg){
      DEBUG_1V("Command processing for device ");DEBUG_1L(devDesc);
      if(!isItemValidCommand(msg->pin))return;
      DEBUG_1V("Pin ");DEBUG_1V(msg->pin);DEBUG_1L(" is valid");
      short newVal = *(short*)msg->val;
      short oldVal = getVal(msg->pin);
      if(oldVal != newVal){
        DEBUG_1V("Changing value ");DEBUG_1V(newVal);DEBUG_1V(" type: ");DEBUG_1L(msg->valType);
        if(msg->valType == Can_message::ValType::T_DIMM){
          setDimmer(msg->pin, oldVal, newVal);
        }else{
          DEBUG_1L("Digital command");
          setHWVal(msg->pin, setVal(msg->pin, newVal, msg->valType),msg->valType);
        }
        nextBroadcastItm = msg->pin;
        nextBroadcast = 0;
      }else{
        DEBUG_1V("Old Val: ");DEBUG_1V(oldVal);DEBUG_1V("New Val: ");DEBUG_1L(newVal);
      }
    };
    void checkInput(int gpio_no){
      short currVal, prevVal;
      if(!isItemValidInput(gpio_no)) return;

      prevVal = getVal(gpio_no);
      currVal = getHWVal(gpio_no);  
      if (millis() - getValSince(gpio_no) > 5000) clearValSince(gpio_no);
      if( currVal != prevVal ){
        if((getValSince(gpio_no) != 0) && (millis() - getValSince(gpio_no) > 50)){
          DEBUG_1V("Push button ");DEBUG_1V(gpio_no);DEBUG_1V(" Old Val: ");DEBUG_1V(prevVal);DEBUG_1V(" New Val: ");DEBUG_1L(currVal);

          setVal(gpio_no, currVal);
          clearValSince(gpio_no);
          nextBroadcastItm = gpio_no;
          nextBroadcast = 0;
          DEBUG_1L("Set button end");
        }else if(getValSince(gpio_no) == 0) 
          setValSince(gpio_no); 
      } 
    };  
}; 

class GPIO_Pins:public PinDevice{
  protected:

    struct single_pin{
      byte  reserved:1,
            initialized:1,
            in_out:2;
      short lastVal;
      unsigned long long valSince;
    };
    single_pin  pins[PC15+1];
    
    bool isItemValidBcast(int);
    bool isItemValidInput(int);
    bool isItemValidCommand(int itm);
    inline int getVal(int itm){ return pins[itm].lastVal; };
    inline int setVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ pins[itm].lastVal = val; return val; };
    inline int getHWVal(int itm){ return digitalRead(itm); };
    void setHWVal(int itm, int val, byte valType = Can_message::ValType::T_INT){      
      if(valType == Can_message::ValType::T_PWM){
        DEBUG_1L("....................Analog................");
        analogWrite(itm, val);
      }else{
        DEBUG_1L(itm);
        digitalWrite(itm, val);
      }
    };
    inline unsigned long long getValSince(int itm){ return pins[itm].valSince; };
    inline void setValSince(int itm){ pins[itm].valSince = millis(); };
    inline void clearValSince(int itm){ pins[itm].valSince = 0; };
    inline bool inputAvailable(){ return true; };
  public:
    GPIO_Pins(STM32* stm);
    // void loop();
    // void canCommand(Can_message::message* msg);
    // void checkInput(int gpio_no);
    
};
#endif
