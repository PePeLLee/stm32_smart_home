import can
import os 
import logging
import canMsg
from threading import RLock
from time import sleep
from queue import Queue, Empty

class CanClient:
    lastMsgQ = Queue()
    def __init__(self, ev = None):
        self.lock  = RLock()
        logging.debug("Can client constructor")
        os.system('sudo ifconfig can0 up')
        self.can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native
        self.event = ev

    def __del__(self):
        print ("Can client destructor")
        os.system('sudo ifconfig can0 down')

    def sendMessage(self, id, data, resend = 0):
        logging.info("Sending CAN message {} to {}".format(data, id))
        msg = can.Message(arbitration_id=id, data=data, extended_id=False)
        try:
            #with self.lock:
            self.can0.send(msg)
            sleep(0.01)
        except OSError:
            logging.warning("Discarding TX buffer")
            self.can0.flush_tx_buffer()
            if(resend == 0):
                self.sendMessage(id, data, 1)

    def popMsg(self):
        try:
            msg = self.lastMsgQ.get_nowait()
            return msg
        except Empty:
            return None
        #try:
            #msg = self.lastMsg
            #del self.lastMsg
            #return msg
        #except AttributeError:
        #    return None

    def loop(self):
        while True:
            #with self.lock:
            #logging.debug("CAN loop")
            msg = self.can0.recv()
            if msg:
                #logging.info("CAN message received----")
                try:
                    #self.lastMsg = canMsg.CanMsg(msg.data, msg.arbitration_id)
                    msgCnf = canMsg.CanMsg.fromByteArray(msg.arbitration_id, msg.data)
                    self.lastMsgQ.put(msgCnf)
                    #try:
                        #msgCnf = msgCnf.toConfimation()
                        #self.sendMessage(msgCnf.sender, msgCnf.build())
                    #except ValueError as err:
                    #    logging.error("Error preparing confirmation msg {} {} from {}".format(msg.data, err, msg.arbitration_id))
                    #except AttributeError as err:
                    #    logging.error("Error preparing confirmation msg {} {} from {}".format(msg.data, err, msg.arbitration_id))

                    if self.event: 
                        self.event.set() 
                except ValueError as err:
                    logging.error("Error parsing msg {} {} from {}".format(msg.data, err, msg.arbitration_id))


if __name__ == "__main__":
    logging.debug("CAN client test")
    clnt = CanClient()
    clnt.sendMessage(12, [0x26, 0,0,0,0,1,0,0])

