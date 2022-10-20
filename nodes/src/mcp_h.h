#ifndef MCP_H_H
#define MCP_H_H
class McpPin: public Pin{
	Adafruit_MCP23017* mcp;	
	public:
	void changeValue(byte val);
	void checkInput();
	
	
	McpPin(int i, int in_out, STM32* stm, Adafruit_MCP23017* mcp, int seq):Pin(stm){

		mcp->pinMode(i, in_out);
    this->lastVal = HIGH;
    
    this->in_out = in_out;
		if(in_out == OUTPUT){
//      this->lastVal = HIGH;
      mcp->digitalWrite(i, this->lastVal);
      this->devNo = Can_message::Device::MCP20 + seq;
      sprintf(devDesc, "MCP Out");
    }else{      
      mcp->pullUp(i, HIGH);
      sprintf(devDesc, "MCP In");
      this->devNo = Can_message::Device::MCP20 + seq - 1 ;
		} 
		this->gpio_no = i;
		this->mcp = mcp;
	};
};

class McpExtender{
		Adafruit_MCP23017 mcp;
		STM32* stm;
		int seq;
    int in_out;
		McpPin* pins[16];
	public:
		McpExtender(STM32* stm, int seq = 0){
			in_out=(seq<4)?OUTPUT:INPUT;//_PULLUP;
			mcp.begin(seq);
			this->stm = stm;
      this->seq = seq;
      
			for(int i=0; i<16; i++)
					pins[i] = new McpPin(i, in_out, stm, &mcp, seq);
		}
		void checkInputs();
    void loop();
    void canCommand(Can_message::message* msg);
};

#endif
