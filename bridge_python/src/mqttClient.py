import paho.mqtt.client as mqtt
import logging
import canMsg
from copy import deepcopy
from threading import Lock
from time import sleep
from queue import Queue, Empty



class MqttMessage:
    keepAlive = False
    dev        = 0

    def parse(self, txt):
        #logging.debug("parsing Mqtt message") 
        parts = str(txt).split("|")
        #['013', '1', 'PA4 ', '0', '0019']
        self.senderId = parts[0].strip()
        self.msgType = parts[1].strip()
        self.pin = parts[2].strip()
        self.dev = 0
        if(self.pin[0:2]=="M0"):
            self.dev=1
            self.pin = self.pin[2:]
        if(self.pin[0:2]=="P0"):
            self.dev=12
            self.pin = self.pin[2:]
        if(self.pin[0:2]=="P1"):
            self.dev=13
            self.pin = self.pin[2:]
        self.valTyp = parts[3].strip()
        self.val = parts[4].strip()
        if(len(parts)>5):
            self.type = parts[5].strip()
            if(len(parts)>6):
                self.dev = parts[6].strip()
        else:
            self.type = 1 


    def fromCanMsg(self, msgCan):
        self.senderId   = msgCan.sender
        if(msgCan.msgArr ==  b'\x11\x12\x13\x11\x12\x13\x11\x12'):
            logging.debug("Keepalive from {}".format(msgCan.sender)) 
            self.keepAlive = True
            self.msgType    = canMsg.MsgType.WELCOM
            self.val        = 0
            self.dev        = 0
        else:
            #logging.debug("{}".format(msgCan.toString())) 
            self.msgType    = canMsg.MsgType(msgCan.type)
            self.dev        = msgCan.dev
            if(self.dev == 0):
                self.pin        = canMsg.Pin(msgCan.pin).name
            else:
                self.pin        = msgCan.pin

            self.valTyp     = msgCan.valTp
            self.val        = msgCan.val

    def concat(self):
        #012|2|PB12|0|0001
        try:
            if self.keepAlive == True:
                txt = "Keepalive from {}".format(self.senderId)
            else:
                txt = "{:03}|{}|{}|{:<4}|{}|{:04}".format(
                        self.senderId, 
                        self.msgType.value,
                        self.dev,
                        self.pin,
                        self.valTyp,
                        self.val
                    )
        except AttributeError:
            txt = 'Unknown message from {}'.format(self.senderId)
        return txt

    def newTopic(self):
        try:
            if self.keepAlive == False:
                return "dzialka/bridge/{}/{}/{}/{}/{}".format(self.senderId, self.dev, self.msgType.value, self.pin, self.valTyp)
            else:
                return "dzialka/bridge/{}/{}".format(self.senderId, self.msgType.value)
        except AttributeError:
            return "dummy"

    def newTopicBare(self):
        try:
            if self.keepAlive == False:
                return "dzialka/bridge/{}/{}/{}/{}/{}".format(self.senderId, self.dev, self.msgType.value, type(self.pin), self.valTyp)
            else:
                return "dzialka/bridge/{}/{}".format(self.senderId, self.msgType.value)
        except AttributeError:
            return "dummy"


    def newMsg(self):
        try:
            return self.val
        except AttributeError:
            return "dummy"


    def toString(self):
        str = "Sender Id: {}\n".format(self.senderId)
        str = "{} Message Type: {}\n".format(str, self.msgType)
        str = "{} Pin: {}\n".format(str, self.pin)
        str = "{} Dev: {}\n".format(str, self.dev)
        str = "{} Value Type: {}\n".format(str, self.valTyp)
        str = "{} Value: {}\n".format(str, self.val)
        str = "{} Type: {}\n".format(str, self.type)
        return str

class MqttClient:
    lastMsgQ = Queue()
    def __init__(self, topic, event = None):
        self.client = mqtt.Client()
        self.client.username_pw_set("mqtt", password="mqtt")
        self.client.connect("192.168.7.249", 1883, 60)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.topic = topic
        self.event = event

    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        client.subscribe(self.topic)

    def on_message(self, client, userdata, msg):
        logging.debug("Received MQTT TOPIC: {}, PAYLOAD: {}".format(msg.topic, str(msg.payload)))
        mmsg =  MqttMessage()
        mmsg.parse(msg.payload.decode('utf-8'))
        logging.debug(mmsg.toString())
        self.lastMsgQ.put(mmsg)
        if self.event:
            self.event.set()
            
        #dzialka/bridge/st 012|1|PA4 |0|0019
        #dzialka/bridge/ 014|2|PB4 |0|0001
        #dzialka/bridge/ 014|2|PB5 |0|0001
        #dzialka/bridge/st 014|1|PA4 |0|0019

    def publish(self, mqttMsg):
        if mqttMsg.msgType == canMsg.MsgType.COMMAND:
            logging.debug("Command - not resending")
        else:
            logging.info("Publishing mqtt message {}".format(mqttMsg.concat()))
            self.client.publish("dzialka/bridge/st", mqttMsg.concat())
            logging.info("Publishing topic {}".format(mqttMsg.newTopic()))
            self.client.publish(mqttMsg.newTopic(), mqttMsg.newMsg())

            if mqttMsg.dev != 0:
                logging.info("Publishing bate topic {}".format(mqttMsg.newTopicBare()))
                #self.client.publish(mqttMsg.newTopicBare(), mqttMsg.newMsg())

    def popMsg(self):
        try:
            msg = self.lastMsgQ.get_nowait()
            return msg
        except Empty:
            return None

        
    def loop(self):    
        self.client.loop_start()

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    logging.debug("Mqtt message test")
    mqttCli = MqttClient("dzialka/bridge2/")
    mqttCli.loop()


