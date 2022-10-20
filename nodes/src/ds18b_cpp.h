#include "board/board.h"
#include "wirish_time.h"
#include "usb_serial.h"
#ifndef DS18B_CPP_H
#define DS18B_CPP_H

#include "ds18b_h.h"

int Ds18b::getAddr(int itm)
{  
  return meters[itm].addr[6]*16+meters[itm].addr[7];
};

char* Ds18b::addrToStr(DeviceAddress addr){
  sprintf(this->buff, "");
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (addr[i] < 16) sprintf(this->buff,"%s0",this->buff);
    sprintf(this->buff,"%s%X",this->buff,addr[i]);
  }
  return this->buff;
}
  
int Ds18b::getTemp(int itm)
{
  float tempC = sensors->getTempC(meters[itm].addr);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    #ifdef DEBUG_2
      Serial.print(addrToStr(meters[itm].addr));
      Serial.print("    ");
      Serial.println("disconnected");
    #endif
    return 0;
  }
  #ifdef DEBUG_2
    Serial.print(addrToStr(meters[itm].addr));
    Serial.print("    ");
    Serial.println(tempC);
  #endif
  meters[itm].lastVal = (int)(tempC*10); 
  return meters[itm].lastVal; 
};

Ds18b::Ds18b(STM32* stm):Device(stm){
  sprintf(devDesc, "Ds18b");
  this->broadcastDelay = 20000;
  this->refreshDelay = 60000;
  oneWire= new OneWire(WIRE_PIN);
  sensors = new DallasTemperature(oneWire);
  sensors->begin();
  this->maintenance(NULL);
};


void Ds18b::maintenance(Can_message::message* msg){
  #ifdef DEBUG_2
        char tmp[30];
        sprintf(tmp, "DS18 maintenance ");
        Serial.println(tmp);
        delay(10);
  #endif
  this->totalItems=0;
  oneWire->reset_search();
  while(oneWire->search(meters[this->totalItems++].addr)){
    #ifdef DEBUG_2
      Serial.println(addrToStr(meters[this->totalItems-1].addr));
    #endif
  };
  this->totalItems--;
  for(int i=0; i<this->totalItems; i++){
    sensors->setResolution(meters[i].addr, 11);
  }
  #ifdef DEBUG_2
    sprintf(tmp, "DS18 count %d ", sensors->getDeviceCount());
    Serial.println(tmp);
    sprintf(tmp, "DS18 searched %d ", this->totalItems);
    Serial.println(tmp);
    delay(10);
  #endif
};

void Ds18b::refreshData(){
    sensors->requestTemperatures();  
};

void Ds18b::broadcastItemState(int itm){
  #ifdef DEBUG_2
    char tmp[40];
    sprintf(tmp, "Broadcast temperature %d ", millis());
    Serial.println(tmp);
  #endif
  stm->broadcastInputChange(getAddr(itm), getTemp(itm), Can_message::Device::DS18B);
  #ifdef DEBUG_2
    sprintf(tmp, "Broadcast end temperature %d ", millis());
    Serial.println(tmp);
  #endif
};
		

#endif
