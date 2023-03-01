Can_communication::Can_communication(mcp2515_can *CAN0, STM32*stm){
	this->can = CAN0;
	this->stm = stm;
	this->msg = new Can_message( stm );
};
		
void Can_communication::keepAlive(){
	if (millis() - lastKeepAlive > 5000){
		byte tst[8]={0x11, 0x12, 0x13, 0x11, 0x12, 0x13, 0x11, 0x12};
		can->sendMsgBuf(DEV_ID, 0, 8, tst);
		lastKeepAlive = millis();  
	}
};

void Can_communication::sendInputState(int pin, int val, int attNo, int device){
	
	if(!initSucc){
		Serial.println("No communication established");
		return;	
	}
	byte canBuff[8];
	memset(canBuff, 0, 8);
	Can_message::message* outMsg = (Can_message::message*) canBuff;
	outMsg->type = Can_message::MsgType::STATUS;
	outMsg->pin = pin;
	outMsg->device = device;
	outMsg->valType = attNo;
	// outMsg->val[1] = val;  
  *(short*)outMsg->val = (short)val; 
	can->sendMsgBuf(DEV_ID, 0, 8, canBuff);
	
	if(can->checkError() == CAN_OK){
	} else {
	}
};
#ifdef RF_RECEIVE
void Can_communication::sendRfButton(int id){
  if(!initSucc){
    Serial.println("No communication established");
    return;
  }
  byte canBuff[8];
  memset(canBuff, 0, 8);
  Can_message::message* outMsg = (Can_message::message*) canBuff;
  outMsg->type = Can_message::MsgType::STATUS;
  outMsg->pin = 0;
  outMsg->device = 15;
  outMsg->valType = 0;
  *(short*)outMsg->val = (short)id ;  
  can->sendMsgBuf(DEV_ID, 0, 8, canBuff);
};

void Can_communication::sendClimate(byte mac, int type, short val){
  if(!initSucc){
    Serial.println("No communication established");
    return;
  }
  byte canBuff[8];
  memset(canBuff, 0, 8);
  Can_message::message* outMsg = (Can_message::message*) canBuff;
  outMsg->type = Can_message::MsgType::STATUS;
  outMsg->pin = mac;
  outMsg->device = 15;
  outMsg->valType = type;
  *(short*)outMsg->val = (short)val ;  
  can->sendMsgBuf(DEV_ID, 0, 8, canBuff);
};
#endif
	
void Can_communication::checkMessage(){
	char msgString[128];  
  if(initSucc && (CAN_MSGAVAIL == can->checkReceive())){
//		Serial.println("Any message spotted");
    // Serial.print(".");
		can->readMsgBuf(&len, rxBuf);      // Read data: len = data
		rxId = can->getCanId();
		if (rxId != DEV_ID) { 
      // Serial.print("-");
			return; 
		}
		if(len<2){ Serial.println("Message too short"); return;	} 
    Serial.print("+");
    if((Can_message::MsgType)((Can_message::message*)rxBuf)->type == Can_message::MsgType::WELCOME){
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
      Serial.println();
    
    }
		stm->blink = millis();
		msg->print((Can_message::message*)rxBuf);
		msg->process((Can_message::message*)rxBuf);
	}
};
