/*
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <EthernetENC.h>
#include <PubSubClient.h>
#include <mcp2515_can.h>


EthernetClient ethClient;
PubSubClient client(ethClient);
mcp2515_can can(PA0); 
  
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

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 7, 203);
IPAddress server(192, 168, 7, 249);

		long unsigned int rxId;
		unsigned char len = 0;
		unsigned char rxBuf[8];

message* msg;
char topic[20];
char mess[25];

void checkCan(){

  while((CAN_MSGAVAIL == can.checkReceive())){
//		Serial.println("Any message spotted");
    // Serial.print(".");
		can.readMsgBuf(&len, rxBuf);      // Read data: len = data
    rxId = 5;
		rxId = can.getCanId();
		if(len<2){ Serial.println("Message too short"); return;	} 
    Serial.print("+");
    msg = (message*)rxBuf;
    sprintf(topic, "test/%d", rxId);
    Serial.println(topic);
    sprintf(mess, "%d/%d/%d/%d/%d/%d/%d/%u[%d]", msg->msgId, msg->ver, msg->from_node,
                              msg->type, msg->device, msg->pin, msg->valType, *(short*)msg->val,//);//
                                                                              msg->val[0]*256+msg->val[1]);
    client.publish(topic,mess);
    // if((Can_message::MsgType)((Can_message::message*)rxBuf)->type == Can_message::MsgType::WELCOME){
    //   for(byte i = 0; i<len; i++){
    //     sprintf(msgString, " 0x%.2X", rxBuf[i]);
    //     Serial.print(msgString);
    //   }
    //   Serial.println();
    
    // }
		// stm->blink = millis();
		// msg->print((Can_message::message*)rxBuf);
		// msg->process((Can_message::message*)rxBuf);
	}
}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void reconnect() {
  while (!client.connected()) {
    Serial.println(Ethernet.localIP());
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", "mqtt", "mqtt")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(57600);
  pinMode(PA0, OUTPUT);
  pinMode(PA4, OUTPUT);
  digitalWrite(PA0, HIGH);
  digitalWrite(PA4, HIGH);
  Ethernet.begin(mac);

  client.setServer(server, 1883);
  client.setCallback(callback);

  while(can.begin(CAN_500KBPS) != CAN_OK){

		Serial.println("Retry MCP2515 Init!");
    delay(100);
		// can.initSucc = 1;
    // CAN0->init_Mask(0, 0, 0x3ff);                         // there are 2 mask in mcp2515, you need to set both of them
    // CAN0->init_Mask(1, 0, 0x3ff);
    // CAN0->init_Filt(0, 0, DEV_ID); 
	}
	Serial.println("MCP2515... Initialized");


  // Allow the hardware to sort itself out
  delay(1500);
}

long long nextKeep = 0;
void keepalive(){
  if (millis() > nextKeep) {
    Serial.println("keepalive");
    client.publish("test/keepalive","hello world");
    nextKeep = millis()+10000;
  }
};

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  checkCan();
  keepalive();
}
