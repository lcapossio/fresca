# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile
# ARDUINO_DIR flag HAS TO BE set with the absolute path to you Arduino root directory e.g. /home/some_user/arduino-1.8.5

USER_LIB_PATH = ../lib
ARDUINO_LIBS  = DFR_Key TM1637-1.1.0_7seg LiquidCrystal EEPROM OneWire ds1820 \
                TempController Adafruit_Sensor DHT-sensor-library fresca_sensor fresca_utils fresca

# --- mega2560 ide 1.6
BOARD_TAG    = mega
BOARD_SUB    = atmega2560
MONITOR_PORT = /dev/ttyACM0
# ARDUINO_DIR  = /usr/share/arduino

# Check for ARDUINO_DIR flag
ifndef ARDUINO_DIR
  $(error ARDUINO_DIR is not set, this flag has to be set with the absolute path to you Arduino root directory e.g. ARDUINO_DIR=/home/some_user/arduino-1.8.5)
endif

#Now include the Arduino-Makefile
include ../../subm/Arduino-Makefile/Arduino.mk