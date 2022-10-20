import logging
import mqttClient
import threading
import canMsg
import canClient

FORMAT = '%(asctime)-15s %(levelname)-8s %(message)s'
#logging.basicConfig(format=FORMAT, level=logging.DEBUG)
logging.basicConfig(format=FORMAT, level=logging.ERROR)
#logging.FileHandler('/tmp/spam.log')
#fh.setLevel(logging.)

class Broker:
    event = threading.Event()
    def __init__(self):
        self.mqtt = mqttClient.MqttClient("dzialka/bridge/", self.event)
        self.canClnt = canClient.CanClient(self.event)

    def handler(self):
        logging.debug("handler")
        #self.lock.acquire()
        while True:
            msgMqttR = self.mqtt.popMsg()
            msgCanR  = self.canClnt.popMsg()
            if msgMqttR:
                logging.debug("Broker parsing MQTT message")
                msgCan = canMsg.CanMsg.constr([], int(msgMqttR.senderId))
                msgCan.type = int(msgMqttR.msgType)
                msgCan.valTp = int(msgMqttR.valTyp)
                msgCan.val  = int(msgMqttR.val)
                msgCan.dev  = int(msgMqttR.dev)
                if(msgCan.dev == 0):
                    msgCan.pin  = canMsg.Pin[msgMqttR.pin].value
                else:
                    msgCan.pin = int(msgMqttR.pin)
                #logging.debug("Can MSG: {}".format(msgCan.toString()))
                msgCan.build()
                self.canClnt.sendMessage(int(msgMqttR.senderId), msgCan.msgArr)
                if not msgCan.nodeWillConfirm():
                    msgCan.makeConfirmation()
                    msgMqtt = mqttClient.MqttMessage() 
                    msgMqtt.fromCanMsg(msgCan)
                    self.mqtt.publish(msgMqtt)


            if msgCanR:
                #logging.debug("Broker parsing CAN message")
                msgMqtt = mqttClient.MqttMessage()
                msgCanR.parse()
                #logging.info(msgCanR.msgArr)
                msgMqtt.fromCanMsg(msgCanR)
                self.mqtt.publish(msgMqtt)
            if msgCanR == None and msgMqttR == None:
                self.event.wait(1)
                self.event.clear()
            else:
                logging.debug("There was something in handler loop")
        #logging.debug("end of handler loop")
        #self.lock.release()


    def loop(self):
        self.mqtt.loop()
        threadC = threading.Thread(target=self.canClnt.loop, daemon=True)
        #threadC = threading.Thread(target=self.canClnt.loop, daemon=False)
        threadC.start()
        self.handler()
        #threadM.join()
        threadC.join()

if __name__ == "__main__":
    logging.debug("Broker test")
    broker = Broker()
    broker.loop()


