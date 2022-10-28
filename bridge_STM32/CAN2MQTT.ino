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
  short val;  
};

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 7, 203);
IPAddress server(192, 168, 7, 249);

		long unsigned int rxId;
		unsigned char len = 0;
		unsigned char rxBuf[8];

message* msg;
char topic[100];
char mess[25];

void checkCan(){

  while((CAN_MSGAVAIL == can.checkReceive())){
		can.readMsgBuf(&len, rxBuf);      // Read data: len = data
    rxId = 5;
		rxId = can.getCanId();
		if(len<2){ Serial.println("Message too short"); return;	} 
    msg = (message*)rxBuf;
    sprintf(topic, "canStatus/%d/%d/%d/%d/%d/%d/%d/%d", rxId, msg->msgId, msg->ver, msg->from_node,
                              msg->type, msg->device, msg->pin, msg->valType);
    sprintf(mess, "%u", (short)msg->val);
    client.publish(topic,mess);
	}
}

char tmp[200];
char* msgToStr(int id, message *msg){
  sprintf(tmp,"txId: %d,	msgId: %d, ver: %d, from_node: %d, type: %d, device: %d, pin: %d,	valType: %d, val %d",
               id,       msg->msgId, 
                                   msg->ver, msg->from_node, msg->type, 
                                                                       msg->device, 
                                                                                   msg->pin, 
                                                                                           msg->valType, msg->val);
  return tmp;
};


void callback(char* topic, byte* payload, unsigned int length) {
  message *msg = (message *)&rxBuf;
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");

  // Serial.println();
  char *p = strtok(topic, "/");//topic
  if(!p)return;
  p = strtok(NULL, "/");
  if(!p)return;
  rxId = atoi(p);  p = strtok(NULL, "/"); if(!p)return;
  msg->msgId = atoi(p); p = strtok(NULL, "/"); if(!p)return;
  msg->ver = atoi(p); p = strtok(NULL, "/"); if(!p)return;
  msg->from_node = atoi(p); p = strtok(NULL, "/"); if(!p)return;
  msg->type = atoi(p); p = strtok(NULL, "/"); if(!p)return;
  msg->device = atoi(p); p = strtok(NULL, "/"); if(!p)return;
  msg->pin = atoi(p); p = strtok(NULL, "/"); if(!p)return;
  msg->valType = atoi(p);
  msg->val = 0;//atoi((char*)payload);
  for (int i=0;i<length;i++) {
    // Serial.println(payload[i]-'0');
    // Serial.println(msg->val);
    if(payload[i]>='0' && payload[i]<= '9'){
      msg->val = msg->val*10 + (payload[i]-'0'); 
    }else{
      break;
    }
  }
  can.sendMsgBuf(rxId, 0, 8, rxBuf);
}


void reconnect() {
  byte cnt = 0;
  while (!client.connected()) {
    if(cnt>5)digitalWrite(PB10, LOW);
    Serial.println(Ethernet.localIP());
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", "mqtt", "mqtt")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("canStatus/0/0/","0");
      // ... and resubscribe
      client.subscribe("canCommand/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
    cnt++;
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
