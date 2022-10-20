from enum import Enum
import logging
import random
import ctypes 

class MsgType(Enum):
    WELCOM = 0
    STATUS = 1
    COMMAND = 2
    LED_SI = 3
    _00004 = 4
    _00005 = 5
    _00006 = 6

class Pin(Enum):
    KEEPALIVE = 18
    M000 =  0
    M001 =  1
    M005 =  5
    M006 =  6
    M007 =  7
    M011 =  11
    M012 =  12
    M013 =  13
    M014 =  14
    PB12 =  28 
    PB13 =  29 
    PB14 =  30 
    PB15 =  31 
    PA8  =  8  
    PA9  =  9  
    PA10 =  10 
    PA15 =  15 
    PB3  =  19 
    PB4  =  20 
    PB5  =  21 
    PB6  =  22 
    PB7  =  23 
    PB8  =  24 
    PB9  =  25 
    PB11 =  27 
    PB10 =  26 
    PB1  =  17 
    PB0  =  16 
    PA4  =  4  
    PA3  =  3  
    PA2  =  2  
    PC15 =  34 
    PC14 =  33 
    PC13 =  32
    E100 =  100
    E101 =  101
    E102 =  102
    E103 =  103
    E104 =  104
    E105 =  105
    E106 =  106
    E107 =  107
    E108 =  108
    E109 =  109
    E110 =  110
    E111 =  111
    E112 =  112
    E113 =  113
    E114 =  114
    E115 =  115
    E200 =  116
    E201 =  117
    E202 =  118
    E203 =  119
    E204 =  120
    E205 =  121
    E206 =  122
    E207 =  123
    E208 =  124
    E209 =  125
    E210 =  126
    E211 =  127
    E212 =  128
    E213 =  129
    E214 =  130
    E215 =  131
    R100 =  200
    R101 =  201
    R102 =  202
    R103 =  203
    R104 =  204
    R105 =  205
    R106 =  206
    R107 =  207
    R108 =  208
    R109 =  209
    R110 =  210
    R111 =  211
    R112 =  212
    R113 =  213
    R114 =  214
    R115 =  215
    RSET =  255

class ValType(Enum):
    INT = 0
    FLOAT = 1
    PWM = 2
    ANIM = 3
    _004 = 4

class CanMsg:
    #truct _canMsg{
    #       int type:2;//0 - welcome
    #       //1 - status
    #       //2 - command
    #       int pin:6;//6 bits
    #       int valTyp:1;//0 - int
    #       //1 - float
    #       byte val[4];
    #       } ;
    def __init__(self, msgArr = [], sender = 0):
        logging.debug("CanMsg Class Constructor")
        self.msgArr = msgArr
        self.sender = sender
        self.dev = 0
        self.msgId  = 0 
        self.ver    = 0
        self.fromNode = 0
        self.type   = 0
        self.pin    = 0
        self.valTp  = 0
        self.val    = 0
        if len(msgArr) > 0:
            logging.debug("Size of array = %d",  len(self.msgArr))
            self.parse()

    def toString(self):
        str = ""
        for x in self.msgArr:
            str = "{} 0x{:02X}".format(str, x)
        str += "\n"
        str =  "{} Type:        {} {}".format( str, self.type, MsgType(self.type).name )
        str =  "{} Pin:         {} {}".format( str, self.pin, Pin(self.pin).name ) 
        try:
            str =  "{} Pin:         {} {}".format( str, self.pin, Pin(self.pin).name ) 
        except ValueError:
            str =  "{} Pin:         {} {}".format( str, self.pin, self.pin ) 

        str =  "{} Value Type:  {} {}".format( str, self.valTp, ValType(self.valTp).name )
        str =  "{} Value:       {}".format( str, self.val )
        str =  "{} Sender:       {}".format( str, self.sender )
        return str

    def parse(self):
        #ogging.debug("Parsing Message")
        self.parseOldFormat()
        #logging.info(self.toString() )
    
    def toConfimation(self):
        cnf = CanMsg(self.msgArr, self.sender)
        cnf.type = MsgType.WELCOM.value
        return cnf
    
    def parseOldFormat(self):
        try:
            self.type   = ( self.msgArr[0] & int('00000011',2) )
            self.pin    = ( self.msgArr[0] & int('11111100',2) ) >> 2
            self.valTp  = ( self.msgArr[1] & int('1',2) )
            self.val    = self.msgArr[5]
        except IndexError:
            logging.error("!!!!!Undefined can msg: {} from {}".format(self.msgArr, self.sender))

    def build(self):
        return self.buildOldFormat();

    def buildOldFormat(self):
        self.msgArr = bytearray(8)
        try:
            self.msgArr[0] = ( self.pin & int('111111',2) ) << 2
            self.msgArr[0] += self.type & int('00000011',2) 
            self.msgArr[1] =  self.valTp & int('1',2) 
            self.msgArr[4] = int(self.val/256) % 256
            self.msgArr[5] = self.val % 256
            self.msgArr[7] = 0xFF
        except ValueError as e:
            logging.error("error while building message {}".format(e))

        #logging.info("Built message 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}".format(
        #    *self.msgArr ))
        return self.msgArr

    def nodeWillConfirm(self):
        return False 

    def makeConfirmation(self):
        self.type = MsgType.STATUS.value

    @staticmethod
    def fromByteArray(sender, data):
        #logging.error("sender {}".format(sender))
        if sender < 16:
            #logging.debug("Old Message placeholder")
            return CanMsg(data, sender)
        else:
            return CanMsg2(data, sender)

    @staticmethod
    def constr(data= [], sender = 0):
        if sender < 16:
            return CanMsg(data, sender)
        else:
            return CanMsg2(data, sender)

class CanMsg2(CanMsg):
    def __init__(self, msgArr = [], sender = 0):
        super().__init__(msgArr, sender)
        self.msgId = random.randrange(255)
        self.ver = 1
        self.fromNode = False
        self.dev = 14

    def parse(self):
        #logging.debug("Parsing NEW can message")
        try:
            self.msgId  = self.msgArr[0]
            self.ver    = ( self.msgArr[1] & int('00011100',2) ) >> 2
            self.fromNode  = ( self.msgArr[1] & int('10000000',2) ) >> 7
            self.type   = ( self.msgArr[2] & int('11110000',2) ) >>4
            self.dev    = ( self.msgArr[3] & int('11110000',2) ) >>4
            self.dev2   = ( self.msgArr[3] & int('00001111',2) ) 
            self.pin    = self.msgArr[4]
            #self.valTp  = ( self.msgArr[5] & int('11110000',2) ) >>4
            self.valTp  = ( self.msgArr[5] & int('1111',2) )
            self.val    = self.msgArr[6] * 256 + self.msgArr[7]
            if self.dev == 15 or self.sender >= 24:
                self.val    = self.msgArr[7] * 256 + self.msgArr[6]
                if self.valTp > 0:
                   self.val = ctypes.c_int16(self.val).value
        except IndexError:
            logging.error("!!!!!Undefined can msg: {} from {}".format(self.msgArr, self.sender))
        logging.info(self.toString() )

    def toString(self):
        str = ""
        for x in self.msgArr:
            str = "{} 0x{:02X}".format(str, x)
        str += "\n"
        str =  "{} MsgId: {} |".format( str, self.msgId)
        str =  "{} Ver: {} |".format( str, self.ver)
        str =  "{} FromNode: {} |".format( str, self.fromNode)
        str =  "{} Type: {} {} |".format( str, self.type, MsgType(self.type).name )
        str =  "{} Device: {} |".format( str, self.dev )
        str =  "{} Device2: {} |".format( str, self.dev2 )
        try:
            str =  "{} Pin: {} {} |".format( str, self.pin, Pin(self.pin).name ) 
        except ValueError:
            str =  "{} Pin: {} {} |".format( str, self.pin, self.pin ) 
        str =  "{} Value Type: {} {} |".format( str, self.valTp, ValType(self.valTp).name )
        str =  "{} Value: {} |".format( str, self.val )
        str =  "{}      Sender: {}".format( str, self.sender )
        return str


                                                                                              
    #byte    msgId;
    #byte    :2, ver:3,
    #        :2, from_node:1;    //message from node or to node
    #MsgType :4, type:4;
    #byte    :8;
    #byte    pin:8;
    #byte    :4, valType:4;//0 - int //1 - float
    #byte    val[2];  

    def build(self):
        self.msgArr = bytearray(8)
        try:
            self.msgArr[2] = (self.type & int('00001111',2)) <<4 
            self.msgArr[3] = (self.dev & int('00001111',2)) <<4 
            self.msgArr[4] = self.pin %256 
            self.msgArr[5] =  self.valTp & int('00001111',2) 
            if self.sender >= 24:
                self.msgArr[7] = int(self.val/256) % 256
                self.msgArr[6] = self.val % 256
            else:
                self.msgArr[6] = int(self.val/256) % 256
                self.msgArr[7] = self.val % 256
        except ValueError as e:
            logging.error("error while building message {}".format(e))

        #logging.info("Built message 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}".format(
        #    *self.msgArr ))
        return self.msgArr




if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    print("Test script of CanMsg Module")
    msg = CanMsg( )
    print("59 00 00 00 00 00 00 00")
    msgTxt = bytearray([0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    msg = CanMsg( (msgTxt) )
    msg.buildOldFormat()
    print("72 00 00 00 00 01 00 00")
    msgTxt = bytearray([0x72, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00])
    msg = CanMsg( msgTxt )
    msg.buildOldFormat()


