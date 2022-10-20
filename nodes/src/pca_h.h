#ifndef PCA_H_H
#define PCA_H_H
class PcaPin: public Pin{
  Adafruit_PWMServoDriver *pwm;
  
  public:  
  
  PcaPin(int i, STM32* stm, Adafruit_PWMServoDriver *pwm,int seq):Pin(stm){
    this->pwm = pwm;
    this->pwm->setPin(i, 0);
    this->lastVal = 0; 
    this->gpio_no = i;
    this->seq = seq;
    this->devNo = Can_message::Device::PCA00 + seq;
    sprintf(devDesc, "PCA");
  };
  
  void changeValue(int val){
      this->pwm->setPin(gpio_no, val); 
      this->lastVal = val; 
      this->nextStatus =0;
  }
};


class PcaExtender{
    Adafruit_PWMServoDriver *pwm;
    STM32* stm;
    int seq;
    PcaPin* pins[16];
  public:
    PcaExtender(STM32* stm, int seq = 0){
      this->stm = stm;
      this->seq = seq;
      pwm = new Adafruit_PWMServoDriver(0x40 + seq);
      pwm->begin();
      pwm->setPWMFreq(1000);
      for(int i=0; i<16; i++)
          pins[i] = new PcaPin(i, stm, pwm, seq);
    }
    void loop(){
      for ( int i=0; i< 16; i++){
        pins[i]->checkSendStatus();
      }
    };
    void command(byte pin, byte valType, int val){
      if(pin >= 16){
        Serial.print("Wrong PCA pin number ");
        Serial.println(pin);
        return;
      }
      Serial.print("Switching PWM: ");
      Serial.print(pin);
      Serial.print(" VAL: ");
      Serial.println(val);
      pins[pin]->changeValue(val);
//      this->pwm->setPin(pin, val); 
      //pwm.setPin(0, (rand()%2 == 1)?0:4096); 
    };
};

#endif
