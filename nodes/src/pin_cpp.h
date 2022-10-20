#include "series/stm32.h"
#include "libmaple/gpio.h"
#ifndef PIN_CPP_H
#define PIN_CPP_H

GPIO_Pins::GPIO_Pins(STM32* stm):Device(stm){
  sprintf(devDesc, "GPIO");
  
  broadcastDelay = 2000;
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

void GPIO_Pins::broadcastItemState(int itm){
    stm->broadcastInputChange(itm, pins[itm].lastVal, this->devNo);
};
bool GPIO_Pins::isItemValidBcast(int itm){
  return (pins[itm].reserved == false && 
          pins[itm].initialized == true &&
          Device::isItemValidBcast(itm));
};
void GPIO_Pins::loop(){
  for ( int i=0; i<= PC15; i++){
		if(pins[i].reserved == true) continue;
		checkInput(i);
	}
  Device::loop();
};
    
void GPIO_Pins::maintenance(Can_message::message* msg){};
void GPIO_Pins::canCommand(Can_message::message* msg){
  #ifdef DEBUG_1
    char tmp[30];
    sprintf(tmp, "Pin command: %5d ", msg->pin);
    Serial.println(tmp);
    delay(10);
  #endif
  if(pins[msg->pin].reserved == true){
    #ifdef DEBUG_1
      Serial.println("Pin reserved");
    #endif
    return;
  }
  if(pins[msg->pin].initialized==false || pins[msg->pin].in_out == INPUT){
    #ifdef DEBUG_1
      Serial.println("Initializing PIN");
    #endif  
    pinMode(msg->pin, OUTPUT);
    pins[msg->pin].initialized=true;
    pins[msg->pin].in_out = OUTPUT;
    pins[msg->pin].lastVal = -1;
  }
  short newVal = *(short*)msg->val;
  if(pins[msg->pin].lastVal != newVal){
    #ifdef DEBUG_1
      char tmp[30];
      sprintf(tmp, "Changing value type: %5d ", msg->type);
      Serial.println(tmp);
      delay(10);
    #endif
    pins[msg->pin].lastVal = newVal;
    if(msg->type == Can_message::ValType::T_PWM){
        #ifdef DEBUG_1  
          Serial.println("....................Analog................");
        #endif
          analogWrite(msg->pin, newVal);
       }else{
          digitalWrite(msg->pin, newVal);
          #ifdef DEBUG_1  
            Serial.println(msg->pin);
          #endif
       }
  }else{
    #ifdef DEBUG_1
      char tmp[30];
      sprintf(tmp, "Old Val: %5d New Val: %5d", pins[msg->pin].lastVal, newVal);
      Serial.println(tmp);
      delay(10);
    #endif
  }
}

// char* Pin::toString(){
//   sprintf(tmp,"%s %d %d %d", devDesc, this->gpio_no, this->lastVal, this->devNo);
//   return tmp;  
// }


void GPIO_Pins::checkInput(int gpio_no){
  int currRead;
  if(!this->isItemValidBcast(gpio_no))return;
  if(pins[gpio_no].in_out != INPUT) return;

  currRead = digitalRead(gpio_no);  
  if (millis() - pins[gpio_no].valSince > 5000){
    pins[gpio_no].valSince = 0;
  }
  if( currRead != pins[gpio_no].lastVal ){
    if((pins[gpio_no].valSince != 0) && (millis() - pins[gpio_no].valSince > 50)){
    #ifdef DEBUG_1
      char tmp[50];
      sprintf(tmp, "Push button %d Old Val: %5d New Val: %5d", gpio_no, pins[gpio_no].lastVal, currRead);
      Serial.println(tmp);
      delay(10);
    #endif 

      pins[gpio_no].lastVal = currRead;
      pins[gpio_no].valSince = 0;
      nextBroadcastItm = gpio_no;
      nextBroadcast = 0;
    #ifdef DEBUG_1
      // char tmp[30];
      sprintf(tmp, "Set button end");
      Serial.println(tmp);
      delay(10);
    #endif 

    }else if(pins[gpio_no].valSince == 0){ 
      pins[gpio_no].valSince = millis();
    } 
  } 
}

#endif
