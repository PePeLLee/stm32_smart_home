#include "wirish_time.h"
#include "series/stm32.h"
#include "libmaple/gpio.h"
#ifndef PIN_CPP_H
#define PIN_CPP_H

void PinDevice::broadcastItemState(int itm){
  stm->broadcastInputChange(itm, getVal(itm), this->devNo);
}

void PinDevice::setDimmer(byte pin, short oldV, short newV){
  DEBUG_1L("Starting dimmer");
  if(!isDimmable(pin)){
    setHWVal(pin, setVal(pin, newV));
    return;
  }
  for(int i=0;i<dimmCnt;i++)
    if(dimms[i].pin == pin) return;
  dimms[dimmCnt].pin = pin;
  dimms[dimmCnt].dimmSince = millis();
  dimms[dimmCnt].destVal = (newV>pwmFull())?pwmFull():newV;
  dimms[dimmCnt].initVal = oldV;
  dimmCnt++;
}

#define DIMM_TIME 1000
void PinDevice::handleDimmers(){
  if(dimmCnt == 0) return;
  DEBUG_1L("Dimmer handler");
  //finish dimming
  for(int i = dimmCnt-1 ; i>=0; i--){
    if(millis()>=dimms[i].dimmSince+DIMM_TIME){
      setHWVal(dimms[i].pin, setVal(dimms[i].pin, dimms[i].destVal, Can_message::ValType::T_PWM), Can_message::ValType::T_PWM);
      nextBroadcast = 0;
      nextBroadcastItm = dimms[i].pin;
      dimmCnt--;
    }else{
      break;
    }
  }
  int tPassed;
  for(int i = 0 ; i<dimmCnt; i++){
    tPassed = millis()-dimms[i].dimmSince;
    tPassed = (tPassed>DIMM_TIME)?DIMM_TIME:tPassed;    
    setHWVal(dimms[i].pin,dimms[i].initVal+(dimms[i].destVal-dimms[i].initVal)*tPassed/DIMM_TIME,Can_message::ValType::T_PWM);
  }
}

GPIO_Pins::GPIO_Pins(STM32* stm):PinDevice(stm){
  sprintf(devDesc, "GPIO");
  
  broadcastDelay = 500;
  totalItems = PC15+1;
  
  for(int i=0; i<=totalItems; i++){
    this->pins[i].reserved = true;
    this->pins[i].initialized = false;
    this->pins[i].in_out = UNDEF;
    this->pins[i].lastVal = 1;
  }
  for( int i=0; i< sizeof(availPins)/sizeof(availPins[0]); i++){
		pins[availPins[i]].reserved = false;  
		pins[availPins[i]].initialized = true;
		pins[availPins[i]].in_out = INPUT;
    pins[availPins[i]].valSince = 0;
		pinMode(availPins[i], INPUT_PULLUP);
  }
}

bool GPIO_Pins::isItemValidBcast(int itm){
  return (pins[itm].reserved == false && 
          pins[itm].initialized == true &&
          Device::isItemValidBcast(itm));
};

bool GPIO_Pins::isItemValidInput(int itm) {
  return ( this->isItemValidBcast(itm) && (pins[itm].in_out != OUTPUT));
};
    
bool GPIO_Pins::isItemValidCommand(int itm){
  DEBUG_1V("Pin command: ");DEBUG_1L(itm);
  if(pins[itm].reserved == true){    
    DEBUG_1L("Pin reserved");
    return false;
  }
  if(pins[itm].initialized==false || pins[itm].in_out == INPUT){
    DEBUG_1L("Initializing PIN");
    pinMode(itm, OUTPUT);
    pins[itm].initialized=true;
    pins[itm].in_out = OUTPUT;
    pins[itm].lastVal = -1;
  }
  return true;
};


#endif
