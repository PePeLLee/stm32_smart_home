# STM32 smart home
Private smart home implementation using STM32 bluepill boards with CAN-Bus communication and translation to mqtt.

## Topology
Smart home is set of multiple STM32 bluepill boards connected with 4-wire bus ( in my case it's 4 wires in UTP cable ).
Each microcontroller is connected with required extention modules:
- MCP20_OUT   - first MCP23017 extender for relays (turning light on/off)
- MCP21_OUT   - second MCP23017 extender for relays (turning light on/off)
- ~~MCP24_IN    - first MCP23017 extender for push buttons (wall switches)~~
***MCP23017 should not be used as input over i2c as there is some hardware bug causing i2c freezes***
- PCF1        - first PCF8575 extender for push buttons (wall switches)
- PCF2        - second PCF8575 extender for push buttons (wall switches)
- RF_RECEIVE  - for RF switches
- PCA1        - first PWMServoDriver to controll PWM (transistor driven leds)
- PCA2        - second PWMServoDriver to controll PWM (transistor driven leds)
- DS18        - for reading temperature from oneWire Dallas thermometers
- MODBUS      - for reading MODBUS holding registers from electricity meter / controll air vent

CAN Bus is wired to bridge which translates CAN messages to MQTT messages.
MQTT messages are handled by Home Assistant which is resposible for whole logic. 
