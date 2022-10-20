from enum import Enum
import ctypes 

class MsgType(Enum):
    WELCOM = 0
    STATUS = 1
    COMMAND = 2
    LED_SI = 3

    def __init__(cls, value):
        return value


if __name__ == "__main__":
    val = 65530
    print(ctypes.c_int16(val))
    #try:
    #    MsgType(5)
    #except ValueError:
    #    print("errr")

