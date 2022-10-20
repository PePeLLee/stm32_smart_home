#ifndef MODBUS_CPP_H
#define MODBUS_CPP_H
void modbusPreTransmission(){
  digitalWrite(MAX485_DE, 1);
};
void modbusPostTransmission(){
  digitalWrite(MAX485_DE, 0);
};
Modbus::Modbus(STM32* stm):Device(stm){
  pinMode(MAX485_DE, OUTPUT);
  modbusPostTransmission();
  Serial1.begin(9600,SERIAL_8E1);
  node.begin(1, Serial1);
  node.preTransmission(modbusPreTransmission);
  node.postTransmission(modbusPostTransmission);
  modbusResult = node.ku8MBResponseTimedOut;
  this->broadcastDelay = 2000;
  this->firstAddrBuffered = 0;
  this->totalItems = 0;
  this->addresses[this->totalItems].addr = 0x0E;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 0x10;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 0x12;
  this->addresses[this->totalItems++].multiplier = 10;

  this->addresses[this->totalItems].addr = 22;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 24;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 26;
  this->addresses[this->totalItems++].multiplier = 10;

  this->addresses[this->totalItems].addr = 0x100;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 0x102;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 0x104;
  this->addresses[this->totalItems++].multiplier = 10;
  this->addresses[this->totalItems].addr = 0x106;
  this->addresses[this->totalItems++].multiplier = 10;

  this->addresses[this->totalItems].addr = 0x1C;
  this->addresses[this->totalItems++].multiplier = 100;
  this->addresses[this->totalItems].addr = 0x1E;
  this->addresses[this->totalItems++].multiplier = 100;
  this->addresses[this->totalItems].addr = 0x20;
  this->addresses[this->totalItems++].multiplier = 100;
  this->addresses[this->totalItems].addr = 0x22;
  this->addresses[this->totalItems++].multiplier = 100;

}


void Modbus::maintenance(Can_message::message* msg){
  	// 	byte 	pin:8;
		// byte	valType:4, :4;//0 - int //1 - float
		// byte 	val[2];  
    #ifdef DEBUG_4
      Serial.println();
      Serial.println();
      
      char tmp[30];
      sprintf(tmp, "maintenance val: 0x%X type: %d pin: %d ", *(short*)msg->val, msg->valType, msg->pin);
      Serial.println(tmp);
    #endif
  this->addresses[msg->pin].addr = *(short*)msg->val;
  switch(msg->valType){
    case 0: this->addresses[msg->pin].multiplier = 1; break;
    case 1: this->addresses[msg->pin].multiplier = 10; break;
    case 2: this->addresses[msg->pin].multiplier = 100; break;
  };
  if(msg->pin >= this->totalItems)this->totalItems = msg->pin+1;
  
};

void Modbus::broadcastItemState(int itm){
    #ifdef DEBUG_4
      char tmp[30];
      sprintf(tmp, "Broadcast addr 0x%X ", this->addresses[itm].addr);
      Serial.println(tmp);
      long long int mls = millis();
  #endif
  if(this->addresses[itm].addr > this->firstAddrBuffered + 8 ||
     this->addresses[itm].addr < this->firstAddrBuffered ){
    //read 8 bytes out of modbus starting addr
    #ifdef DEBUG_3
      char tmp[30];
      sprintf(tmp, "Starting addr 0x%X modbus read: %d ", this->addresses[itm].addr, millis());
      Serial.println(tmp);
      long long int mls = millis();
    #endif
    modbusResult = node.readHoldingRegisters(this->addresses[itm].addr, 8);
    Serial1.flush();
    if(modbusResult  == node.ku8MBSuccess)
      this->firstAddrBuffered = this->addresses[itm].addr;
    #ifdef DEBUG_3
      sprintf(tmp, "total time: %d", millis()-mls);
      Serial.println(tmp);
    #endif
  }
  if(modbusResult  == node.ku8MBSuccess){
    int val = (int)(toFloat(this->addresses[itm].addr-this->firstAddrBuffered)*this->addresses[itm].multiplier);
    #ifdef DEBUG_3
      char tmp[30];
      sprintf(tmp, "Broadcast addr: %d value: %d ", this->addresses[itm].addr, val);
      Serial.println(tmp);
    #endif
    stm->broadcastInputChange((byte)this->addresses[itm].addr, val, Can_message::Device::MODBU);
  }else{
    #ifdef DEBUG_3
      char tmp[30];
      sprintf(tmp, "Failed to read addr 0x%X ", this->addresses[itm].addr);
      Serial.println(tmp);
    #endif
  }
};

// void Modbus::loop(){
//   uint8_t result;
//   if(millis()>this->nextUpd){
//       #ifdef DEBUG_3
//         char tmp[30];
//         sprintf(tmp, "Starting modbus read: %d ", millis());
//         Serial.println(tmp);
//         long long int mls = millis();
//       #endif
//     switch(this->portionNo){
//       case 0:    
//         result = node.readHoldingRegisters(0x0E, 6);
//         if (result == node.ku8MBSuccess)
//         {
//           stm->broadcastInputChange(0x00, (int)(toFloat(0x00)*10), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x01, (int)(toFloat(0x02)*10), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x02, (int)(toFloat(0x04)*10), Can_message::Device::MODBU);
//         }
//         break;
//       case 1:
//         result = node.readHoldingRegisters(22, 6);
//         if (result == node.ku8MBSuccess)
//         {
//           stm->broadcastInputChange(0x10, (int)(toFloat(0x00)*10), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x11, (int)(toFloat(0x02)*10), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x12, (int)(toFloat(0x04)*10), Can_message::Device::MODBU);
//         }    
//         break;
//       case 2:
//         result = node.readHoldingRegisters(0x100, 8);
//         if (result == node.ku8MBSuccess)
//         {
//           stm->broadcastInputChange(0x20, (int)(toFloat(0x02)*10), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x21, (int)(toFloat(0x04)*10), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x22, (int)(toFloat(0x06)*10), Can_message::Device::MODBU);
//         }
//       break;
//       case 3:
//         result = node.readHoldingRegisters(0x1C, 8);
//         if (result == node.ku8MBSuccess)
//         {
//           stm->broadcastInputChange(0x30, (int)(toFloat(0x00)*100), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x31, (int)(toFloat(0x02)*100), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x32, (int)(toFloat(0x04)*100), Can_message::Device::MODBU);
//           stm->broadcastInputChange(0x33, (int)(toFloat(0x06)*100), Can_message::Device::MODBU);
//         }
//       break;
//     }
//     this->portionNo = ( this->portionNo + 1 ) % 4;
//     this->nextUpd = millis()+10000;
    
//       #ifdef DEBUG_3
//         // char tmp[30];
//         sprintf(tmp, "End modbus read: %d ", millis());
//         Serial.println(tmp);
        
//         sprintf(tmp, "total time: %d ", millis()-mls);
//         Serial.println(tmp);
//         // long long int mls = millis();
//       #endif
//   }
//   Serial1.flush();
// };

float Modbus::toFloat(int addr){
      union {
        float asFloat;
        uint16_t asInt[2];
      }
      flreg;

      flreg.asInt[1] = node.getResponseBuffer(addr);
      flreg.asInt[0] = node.getResponseBuffer(addr+1);
      return flreg.asFloat;
    };
#endif