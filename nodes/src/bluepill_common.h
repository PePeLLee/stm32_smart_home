#ifndef BLUEPILL_COMMON
#define BLUEPILL_COMMON


#if defined(MCP24_IN) || defined(MCP20_OUT) || defined(PCA1)
	#define I2C
#endif

#ifdef COLOR_LED
  #include "bluepill_led.h"
  Led_strip* led = new Led_strip();
#endif

class STM32;


class Can_communication;

class Can_message{
	
		STM32* stm;
	public:
	Can_message(STM32* stm){
		this->stm = stm;
	}
	enum MsgType{ 
		WELCOME = 0,
		STATUS  = 1,
		COMMAND = 2,
		LED_MOSI = 3,
    MAINT   = 4,
    
	};
	
	enum Device{ 
    STM00 = 0,
		MCP20 = 1,
		MCP21 = 2,
    DS18B = 10,
    MODBU = 11,
    PCA00 = 12,
    PCA01 = 13
	};

	enum ValType{
		T_INT = 0,
		T_FLOAT = 1,
		T_PWM = 2,
    T_DIMM = 3
	};
	
	struct message{
		byte	msgId;
		
		byte	:2,	ver:3,
				:2, from_node:1;	//message from node or to node
		byte    bl1:4, type:4;
		byte    :4, device:4;
		byte 	pin:8;
		byte	valType:4, :4;//0 - int //1 - float
		byte 	val[2];  
	};
	
	void print(message* msg);
	void process(message* msg);
	void status(message* msg);
	void command(message* msg);
  void led_mosi(message* msg);
};  

class Device{
  protected:    
    char devDesc[7];
    STM32* stm;
    unsigned long long nextBroadcast = 0;
    byte nextBroadcastItm = 0;
    unsigned long long nextDataRefresh = 0;
    long broadcastDelay = 1000;
    long refreshDelay = 10000;
    long longBroadcastDelay = 30000;
    byte totalItems = 1;
    byte devNo = 0;
    virtual void broadcastItemState(int) =0;
    virtual void refreshData(){};
    virtual bool isItemValidBcast(int itm ){
      return itm < this->totalItems;
    };

  public:
    Device(STM32* stm){      
      this->stm = stm;
      sprintf(devDesc, "unInit");
    }
    virtual void canCommand(Can_message::message* msg) = 0;
    void loop(){
      byte prev_itm = this->nextBroadcastItm;
      if(millis()>this->nextDataRefresh){
        refreshData();
        this->nextDataRefresh = millis() + this->refreshDelay;
      };
      if(millis()>this->nextBroadcast){
        if(isItemValidBcast(nextBroadcastItm))
          broadcastItemState(nextBroadcastItm);
        this->nextBroadcast = millis() + broadcastDelay;
        do{
          #ifdef DEBUG_11
            char tmp[30];
            sprintf(tmp, "Do While: %d", this->nextBroadcastItm );
            Serial.println(tmp);
            delay(10);
          #endif
          //no check if any item is valid for boradcast!
          this->nextBroadcastItm = (this->nextBroadcastItm + 1) % (this->totalItems + 1);
        }while(!isItemValidBcast(this->nextBroadcastItm));
        if (this->nextBroadcastItm<prev_itm) {
          this->nextBroadcast = millis() + longBroadcastDelay;
        }
      }
    };

    virtual void maintenance(Can_message::message* msg) = 0;
};
#include "pin_h.h"



#if defined(MCP24_IN) || defined(MCP20_OUT)
#include "mcp_h.h"
#endif
#ifdef RF_RECEIVE
#include "rf_receive.h"
#endif
#ifdef PCA1
#include "pca_h.h"
#endif
#ifdef DS18
  #include "ds18b_h.h"
#endif

#ifdef MODBUS
  #include "modbus_h.h"
#endif

class STM32{
	public:
		
#ifdef MCP20_OUT
		int no_mcp_exts_out;
		McpExtender * mcp_exts_out[4];
#endif
#ifdef MCP24_IN
		int no_mcp_exts_in;
		McpExtender * mcp_exts_in[4];
#endif
#ifdef PCA1
		int no_pca_ext;
		PcaExtender * pca_exts[4];
#endif
#ifdef RF_RECEIVE
    RF_receiver *rf_rec;
#endif
#ifdef DS18
  Ds18b *ds18b;
#endif
#ifdef MODBUS
  Modbus* modbus;
#endif
    GPIO_Pins* pins;
		mcp2515_can* CAN0;
		Can_communication* can;
		long long unsigned blink;
		bool blinkOn;
		void setup(mcp2515_can* CAN0);
		void loop();
		void checkInputs();
		void broadcastInputChange(int pin, int val, int device = 0);
		void checkBlink();
    void reboot();
    void maintenance(Can_message::message* msg);
    #ifdef RF_RECEIVE
    void broadcastRfButton(int id);
    void broadcastClimate(byte mac, int type, short val);
    #endif
};


class Can_communication{
	//static v
	public:
	
		mcp2515_can* can;
		STM32* stm;
		Can_message *msg;long long lastKeepAlive = 0;
		byte initSucc = 0;
		
		long unsigned int rxId;
		unsigned char len = 0;
		unsigned char rxBuf[8];
	
		Can_communication(mcp2515_can*, STM32*);
		void keepAlive();
		void checkMessage();
		void sendInputState(int, int, int=0, int=0);
    
    #ifdef RF_RECEIVE
    void sendRfButton(int id);
    void sendClimate(byte mac, int type, short val);
    #endif
};

#ifdef SERIAL_INPUT
class Serial_Input{
	String s;
	public:
	void loop(STM32*);
	void process(STM32*, String);
};
Serial_Input ser;
#endif


#include "stm32_cpp.h"

void Can_message::print(message* msg){
  if((MsgType)msg->type == WELCOME){Serial.print("W");return;}
	Serial.print("!!!Type: ");
	switch((MsgType)msg->type){
	  case WELCOME: Serial.println("Welcome");return;break;
	  case STATUS: Serial.print("Status");break;
	  case COMMAND: Serial.print("Command");break;
    case LED_MOSI: Serial.print("Led Color");break;
    case MAINT:  Serial.print("Maintenance");break;
	  default: Serial.print("Unkn: ");Serial.print((int)msg->type);break;
	}
	Serial.print("BL1: "); Serial.print(msg->bl1);
  Serial.print("Dev: "); Serial.print(msg->device);
	Serial.print("Pin: "); Serial.print(msg->pin);
	Serial.print("Value Type: "); 
	switch((ValType)msg->valType){
    case T_INT: Serial.print("Int");break;
    case T_FLOAT: Serial.print("Float");break;
    case T_PWM: Serial.print("Analog");break;
    default: Serial.print(msg->valType);break;
  };
	Serial.print(" Value: "); Serial.print(*(short*)msg->val);
	 
	Serial.println();
};
void Can_message::process(message* msg){
	switch(msg->type){
	  case WELCOME:  break;
	  case STATUS: 	status(msg);break;
	  case COMMAND: command(msg);break;
#ifdef COLOR_LED
    case LED_MOSI: led_mosi(msg);break;
#endif
    case MAINT: stm->maintenance(msg);break;
	  default: Serial.print("Unsupported message type: ");Serial.println(msg->type);break;
	}
};

void Can_message::status(message* msg){
};


void Can_message::command(message* msg){
  //Serial.println("can command ");
  int val;
  switch (msg->device){
#ifdef MCP20_OUT
    case MCP20:
    #ifdef MCP21_OUT:
      case MCP21:
    #endif
  		this->stm->mcp_exts_out[msg->device - MCP20]->canCommand(msg);
      break;
#endif
#ifdef PCA1
    case PCA00:
  #ifdef PCA2
    case PCA01:
  #endif 
      this->stm->pca_exts[msg->device - PCA00]->canCommand(msg);
      break;
#endif
    case STM00:
      this->stm->pins->canCommand(msg);
      // if(pins[msg->pin] != NULL)
  		//   pins[msg->pin]->command(msg->val, msg->valType);
  	  break;
    default:
      break;
  }
  
};

void Can_message::led_mosi(message* msg){

  int val = ( msg->val[0] << 8 ) + msg->val[1];
  Serial.print("change led color ");
  Serial.println(val);
  #ifdef COLOR_LED
    led->message(val);
  #endif
  
};

#include "can_communication_cpp.h"
#include "pin_cpp.h"

#if defined(MCP24_IN) ||  defined(MCP20_OUT)
#include "mcp_cpp.h"
#endif


#ifdef SERIAL_INPUT
void Serial_Input::loop(STM32* stm){
	char c;
	while (Serial.available() > 0) {
		c = Serial.read();
		if(c=='\n'){
			this->process(stm, s);
			s = "";
			break;
		}
		s += c;
		
	}
}

void Serial_Input::process(STM32* stm, String s){
	char str_c[20];
	strcpy(str_c, s.c_str());
	char * pin_c, * val_c;
	byte pin;
	int val;
	pin_c = strtok(str_c, "|");
	val_c = strtok(NULL, "|");
	pin = atoi(pin_c);
	val = atoi(val_c);
	
	Serial.print("PIN: ");
	Serial.print(" VAL: ");
	Serial.println(val);
	
	stm->pcaCommand(pin, 0, val);
}
#endif

#ifdef RF_RECEIVE
#include "rf_receive_cpp.h"
#endif

#ifdef DS18
  #include "ds18b_cpp.h"
#endif
#ifdef MODBUS
  #include "modbus_cpp.h"
#endif

#endif
