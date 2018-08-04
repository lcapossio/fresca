# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile
# ARDUINO_DIR flag HAS TO BE set with the absolute path to you Arduino root directory e.g. /home/some_user/arduino-1.8.5
# MONITOR_PORT and ISP_PORT have to be set to corresponding USB UART talking to the Arduino board in order to upload/monitor

USER_LIB_PATH = ../lib

BOARD_TAG    = mega
BOARD_SUB    = atmega2560
# MONITOR_PORT = /dev/ttyACM0
# ARDUINO_DIR  = /usr/share/arduino
# MONITOR_BAUDRATE  = 9600

# Check for ARDUINO_DIR flag
ifndef ARDUINO_DIR
  $(error ARDUINO_DIR is not set, this flag has to be set with the absolute path to you Arduino root directory e.g. ARDUINO_DIR=/home/some_user/arduino-1.8.5)
endif

ARDUINO_LIBS  = DFR_Key TM1637 LiquidCrystal EEPROM OneWire ds1820 \
                TempController Adafruit_Sensor DHT-sensor-library

ifdef NOFRESCALINK
#NO fresca link
#define NOFRESCALINK '#define' which should remove fresca-link altogether from the code
CXXFLAGS        += -DNOFRESCALINK='"1"'
else
#Use fresca link
ARDUINO_LIBS    += fresca_link
endif

ARDUINO_LIBS    += fresca_sensor fresca_utils fresca

#Now include the Arduino-Makefile
include ../../subm/Arduino-Makefile/Arduino.mk