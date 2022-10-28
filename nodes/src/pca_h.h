#ifndef PCA_H_H
#define PCA_H_H

#ifdef DEBUG_6 
  #define DEBUG_6V(x) Serial.print(x);
  #define DEBUG_6L(x) Serial.println(x); delay(10);
#else
  #define DEBUG_6V(x)
  #define DEBUG_6L(x)
#endif

	#include <Wire.h>
	#include <Adafruit_PWMServoDriver.h>

// class PcaPin: public Pin{
//   Adafruit_PWMServoDriver *pwm;
  
//   public:  
  
//   PcaPin(int i, STM32* stm, Adafruit_PWMServoDriver *pwm,int seq):Pin(stm){
//     this->pwm = pwm;
//     this->pwm->setPin(i, 0);
//     this->lastVal = 0; 
//     this->gpio_no = i;
//     this->seq = seq;
//     this->devNo = Can_message::Device::PCA00 + seq;
//     sprintf(devDesc, "PCA");
//   };
  
//   void changeValue(int val){
//       this->pwm->setPin(gpio_no, val); 
//       this->lastVal = val; 
//       this->nextStatus =0;
//   }
// };


// class PcaExtender{
//     Adafruit_PWMServoDriver *pwm;
//     STM32* stm;
//     int seq;
//     PcaPin* pins[16];
//   public:
//     PcaExtender(STM32* stm, int seq = 0){
//       this->stm = stm;
//       this->seq = seq;
//       pwm = new Adafruit_PWMServoDriver(0x40 + seq);
//       pwm->begin();
//       pwm->setPWMFreq(1000);
//       for(int i=0; i<16; i++)
//           pins[i] = new PcaPin(i, stm, pwm, seq);
//     }
//     void loop(){
//       for ( int i=0; i< 16; i++){
//         pins[i]->checkSendStatus();
//       }
//     };
//     void command(byte pin, byte valType, int val){
//       if(pin >= 16){
//         Serial.print("Wrong PCA pin number ");
//         Serial.println(pin);
//         return;
//       }
//       Serial.print("Switching PWM: ");
//       Serial.print(pin);
//       Serial.print(" VAL: ");
//       Serial.println(val);
//       pins[pin]->changeValue(val);
// //      this->pwm->setPin(pin, val); 
//       //pwm.setPin(0, (rand()%2 == 1)?0:4096); 
//     };
// };

class PcaExtender:public PinDevice{
  protected:
    Adafruit_PWMServoDriver *pwm;
		int seq;
    struct single_pin{
      short lastVal;
      unsigned long long valSince;
    };
    single_pin  pins[16];
    bool isItemValidInput(int){ return false; };
    inline short pwmFull(){ return 4095; };
    inline bool isItemValidCommand(int itm){ return true; };
    inline int getVal(int itm){ return pins[itm].lastVal; };
    inline int setVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ 
      pins[itm].lastVal = (valType==Can_message::ValType::T_PWM)?val:((val==0)?0:4095); 
      return pins[itm].lastVal;
    };
    inline int getHWVal(int itm){ return -1; };
    void setHWVal(int itm, int val, byte valType = Can_message::ValType::T_INT){ 
      DEBUG_6V("set pwm pin ");DEBUG_6V(itm);DEBUG_6V(" val ");DEBUG_6L(val); 
      this->pwm->setPin(itm, val);
    };
    inline unsigned long long getValSince(int itm){ return pins[itm].valSince; };
    inline void setValSince(int itm){ DEBUG_6L("setValSince"); };
    inline void clearValSince(int itm){ DEBUG_6L("clearValSince"); };
    bool inputAvailable(){return false;};

	public:
		PcaExtender(STM32* stm, int seq = 0):PinDevice(stm){
      pwm = new Adafruit_PWMServoDriver(0x40 + seq);
      pwm->begin();
      pwm->setPWMFreq(1000);
      
      totalItems = 16;
      this->devNo = Can_message::Device::PCA00 + seq;
      sprintf(devDesc, "Pca%d", seq);

      for(int i=0; i<totalItems; i++){
        pins[i].lastVal = 0;
        pwm->setPin(i, pins[i].lastVal);
      }

    };
};

#endif
