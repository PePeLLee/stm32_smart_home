#ifndef MCP_CPP_H
#define MCP_CPP_H

void McpExtender::canCommand(Can_message::message* msg){
  pins[msg->pin]->changeValue((msg->val[1]==0)?LOW:HIGH);
    char tmp[30];
    sprintf(tmp,"%s canCommand %d %d %d", pins[msg->pin]->devDesc, pins[msg->pin]->gpio_no, pins[msg->pin]->lastVal, pins[msg->pin]->seq);
    Serial.println(tmp);
  mcp.digitalWrite(pins[msg->pin]->gpio_no, pins[msg->pin]->lastVal);
}


void McpPin::checkInput(){
	int currRead;
	currRead = mcp->digitalRead(this->gpio_no);  
	if (millis() - this->valSince > 5000){
		this->valSince = 0;
	}
	if( currRead != this->lastVal ){
    

    if((this->valSince != 0) && (millis() - this->valSince > 50)){
      char tmp[30];
      sprintf(tmp,"pin: %d last val %d curVal: %d",this->gpio_no,  this->lastVal, currRead);
      Serial.println(tmp);
			this->lastVal = currRead;
			this->valSince = 0;
      this->nextStatus = 0; //force sending status
		}else if(this->valSince == 0){
			this->valSince = millis();
	   }
   
	}  
		
};

void McpPin::changeValue(byte val){
  this->lastVal = val;
  this->nextStatus = 0;
};


void McpExtender::loop(){
//  if(this->seq == 1){
//      Serial.println(pins[10]->toString());
//  }
    for ( int i=0; i< 16; i++){
      if(in_out!=OUTPUT){
        pins[i]->checkInput();
      }
      pins[i]->checkSendStatus();
    }
}
#endif
