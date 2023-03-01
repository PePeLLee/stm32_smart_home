#include "Arduino.h"
void STM32::setup(mcp2515_can* CAN0){
  disableDebugPorts();
	Serial.begin(115200);
	this->CAN0 = CAN0;
	this->blink = 0;
	this->blinkOn = false;	
#ifdef MCP20_OUT
	mcp_exts_out[0] = new McpExtender(this);
	no_mcp_exts_out = 1;
#endif	
#ifdef RF_RECEIVE
    rf_rec = new RF_receiver(this);
#endif
#ifdef MCP21_OUT
	mcp_exts_out[1] = new McpExtender(this, 1);
	no_mcp_exts_out = 2;
#endif
#ifdef MCP24_IN
  mcp_exts_in[0] = new McpExtender(this, 4);
  no_mcp_exts_in = 1;
#endif
#ifdef PCA1
  pca_exts[0] = new PcaExtender(this);
  no_pca_ext = 1;
#endif
#ifdef PCA2
  pca_exts[1] = new PcaExtender(this, 1);
  no_pca_ext = 2;
#endif
#ifdef DS18
  ds18b = new Ds18b(this);
#endif
#ifdef MODBUS
  modbus = new Modbus(this);
#endif
#ifdef PCF1
  pcf_exts[0] = new PcfExtender(this);
  no_pcf_ext = 1;
#endif
#ifdef PCF2
  pcf_exts[1] = new PcfExtender(this, 1);
  no_pcf_ext = 2;
#endif
#ifdef PCF3
  pcf_exts[2] = new PcfExtender(this, 2);
  no_pcf_ext = 3;
#endif

  pins = new GPIO_Pins(this);

	
	  
	  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
	  
	can = new Can_communication(CAN0, this);
	//if(CAN0->begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK){
	if(CAN0->begin(CAN_500KBPS) == CAN_OK){
		Serial.println("MCP2515 Initialized Successfully!");
		can->initSucc = 1;
    CAN0->init_Mask(0, 0, 0x3ff);                         // there are 2 mask in mcp2515, you need to set both of them
    CAN0->init_Mask(1, 0, 0x3ff);
    CAN0->init_Filt(0, 0, DEV_ID); 
	}else
		Serial.println("Error Initializing MCP2515...");


#ifdef COLOR_LED
  led->setup();
#endif
};
		
void STM32::loop(){
	can->keepAlive();
	can->checkMessage();
  this->checkBlink();
#ifdef MCP20_OUT
	for(int i=0; i<no_mcp_exts_out; i++) mcp_exts_out[i]->loop();//updatePins();
	can->checkMessage();
#endif
#ifdef MCP24_IN
  for(int i=0; i<no_mcp_exts_in; i++) mcp_exts_in[i]->loop();//loop_in();
	can->checkMessage();
#endif
#ifdef PCA1
  for(int i=0; i<no_pca_ext; i++) pca_exts[i]->loop();
	can->checkMessage();
#endif
#ifdef PCF1
  for(int i=0; i<no_pcf_ext; i++) pcf_exts[i]->loop();
	can->checkMessage();
#endif
#ifdef SERIAL_INPUT
	ser.loop(this);
#endif
#ifdef RF_RECEIVE
  rf_rec->loop();
#endif
#ifdef COLOR_LED
    led->loop();
#endif
#ifdef DS18
  this->ds18b->loop();
#endif
#ifdef MODBUS
  this->modbus->loop();
#endif
  pins->loop();
};


void STM32::broadcastInputChange(int pin, int val, int dev){
			this->blink = millis();
#ifdef DEBUG_1
      char tmp[30];
      sprintf(tmp, "Dev: %5d Pin: %5d V: %2d",dev, pin, val);
	    Serial.println(tmp);
      delay(10);
#endif
			can->sendInputState(pin, val, 0,dev);
};

void STM32::broadcastStats(int pin, int val, int dev){
  
			can->sendInputState(pin, val, Can_message::ValType::T_FLIPS,dev);
};
#ifdef RF_RECEIVE
void STM32::broadcastRfButton(int id){
  this->blink = millis();
  Serial.print("button pressed: ");
  Serial.println(id);
  can->sendRfButton(id);
}
void STM32::broadcastClimate(byte mac, int type, short val){
  can->sendClimate(mac, type, val);
}
#endif

void STM32::checkBlink(){
	if(this->blink > 0){
		if (this->blinkOn == false) {
			digitalWrite(PC13, 0);
			this->blinkOn = true;
		}
		if(millis() - this->blink > 500){
			this->blinkOn = false;
			digitalWrite(PC13, 1);
		}		
	}
}
void STM32::reboot(){
  pinMode(PB11, OUTPUT);
  digitalWrite(PB11, LOW);
}

void STM32::maintenance(Can_message::message* msg){
  switch(msg->device){
    case Can_message::Device::STM00:
        if(msg->pin == 0xFF ){
          if(msg->val[1] == 0xFF){
            reboot();
          }  
        };
        break;
#ifdef DS18
    case Can_message::Device::DS18B :
      this->ds18b->maintenance(msg);
      break;
#endif
#ifdef MODBUS
    case Can_message::Device::MODBU :
      this->modbus->maintenance(msg);
      break;
#endif
  }
  
};
