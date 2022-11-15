Can message structure:

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
Example message
"canCommand/26/0/0/2/2/1/10/0" m '0'
"canStatus/devId/msgId/ver/from_node/msgType/device/pin/valType"

Message Types: 

	WELCOME = 0,
	STATUS  = 1,
	COMMAND = 2,
	LED_MOSI = 3,
    MAINT   = 4,
    
Value Types:

	T_INT = 0,
	T_FLOAT = 1,
	T_PWM = 2,
    T_DIMM = 3

Devices numbers:

    STM00 = 0,
	MCP20 = 1,
	MCP21 = 2,
    DS18B = 10,
    MODBU = 11,
    PCA00 = 12,
    PCA01 = 13,
	
put Low on MCP0 (0x20) device pin 10:
mosquitto_pub -h 192.168.7.249 -t "canCommand/26/0/0/2/2/1/10/0" -u mqtt -P mqtt -m '0'
