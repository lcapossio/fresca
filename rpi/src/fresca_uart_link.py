#!/usr/bin/python

#fresca project by Leonardo Capossio

#sys.argv[1], Path to dump log files to
#sys.argv[2], Logging interval in seconds (max 59)


import time,datetime
import serial
import csv
import sys
import fresca_data_log #Data logging
import fresca_logging  #Log files managing

#Commands/responses
START_CMD = int(0xA5);
FLUSH_CMD = int(0x1A);
IDLE_CMD  = int(0xAA);
TEMP_CMD  = int(0xAB); #Temperature readings
HUM_CMD   = int(0xAC); #Humidity readings
CTRLR_CMD = int(0xAD); #Controller status
OK_CMD    = int(0x05);
NOK_CMD   = int(0xFA);

class sensor_class:
  _timestamp=0
  _valueList=[] #Temperature values
  _valueListHum=[] #Humidity values (-1 if no humidity)
  _numSensors=0
  
  def __init__(self, values, timestamp):
    self._valueList  = values
    self._numSensors = len(values)
    self._timestamp  = timestamp
  
  def getTimestamp(self):
    return self._timestamp
  def getHumValues(self):
    return self._valueList
  def getValues(self):
    return self._valueList
  def getValue(self,sensor):
    return self._valueList[sensor]
  def getNumSensors(self):
    return self._numSensors

#8-bit CRC
#Polynomial is taken from Koopman, P. & Chakravarty, T., "Cyclic Redundancy Code (CRC) Polynomial Selection for Embedded Networks," DSN04, June 2004
CRC_POLY=0xA6
def calc_crc(data,crc):
  # for bits in range(8):
    # result = ( data ^ crc ) & 0x01
    # crc = crc >> 1
    # if ( result ):
      # crc = crc ^ CRC_POLY
    # data = data >> 1
  # return crc & 0xFF #Mask output to 8 bits
  return (crc + data) % 256
  
#Parse bytes into 16-bit temperature Q12.4 then into float
def parse_temp(data):
  output=[0.0]*(len(data)/2)
  for i in range(len(data)/2):
    sensor_data = int( (data[(i*2)+1])<<8 | data[i*2] ); #Put together 8-bit data into 16-bits
    if (sensor_data & 0x8000): #Test sign-bit
      #Sign extend
      sensor_data = (int(-1) & (~int(0xffff))) | sensor_data
    
    output[i]=float(sensor_data)/(2**4) #Cast to float and convert from fixed-point
    
  return sensor_class(output,datetime.datetime.today().strftime('%Y%m%d%H%M%S'))
  
def process_temp_packet(byte_buf):

  sensor_data=parse_temp(byte_buf)
  
  for i in range(sensor_data.getNumSensors()):
    print("Sensor"+str(i)+": "+str(sensor_data.getValues()[i]))
  
  return sensor_data

#Receive fresca-link packet
#Reading serial port in binary mode (no text)
#ord() call is really important to turn unicode strings into bytes (I don't know why they made it so difficult)
def rx_packet(port):
    recv = ord(port.read(1))
    if recv == START_CMD:
      print("Got start of packet")
      
      start_time=time.time()
      recv = ord(port.read(1))
      if recv == TEMP_CMD:
        #Temperature command
        #First byte, payload size
        payload_size=ord(port.read(1));
        if payload_size == 0:
          print("***Abort communication, invalid payload")
          return False
        
        print("Payload byte length: "+str(payload_size))
        
        #Receive payload
        rx_buf=[0] * payload_size
        for i in range(payload_size):
          rx_buf[i] = ord(port.read(1));

        #Receive CRC
        rx_crc = ord(port.read(1));
        
        print("UART link took: "+str((time.time()-start_time)*1000))
        
        #Calculate checksum value
        local_crc=0xff
        local_crc=calc_crc(payload_size,local_crc)
        for i in range(payload_size):
          local_crc=calc_crc(rx_buf[i],local_crc)
          
        # for i in range(payload_size):
          # print("Byte "+str(i)+": "+str(rx_buf[i]))

        #Checksum
        if local_crc != rx_crc:
          print("*Wrong checksum, received: "+str(rx_crc)+", calculated: "+str(local_crc))
        else:
          # print("Good frame!!! Checksum rx: "+str(rx_crc)+", calculated: "+str(local_crc))
          return rx_buf
    
    #No packet or wrong checksum
    return False

iter=0
success=0
failure=0
bad_crc=0


if len(sys.argv) != 3:
  print('Too few arguments')
  print('[0]: Path to dump log files to')
  print('[1]: Logging interval')
  quit()
  
log_path=sys.argv[1] #Path to dump log files to
log_interval=int(sys.argv[2]) #Logging interval in seconds (max 59)

#Open serial port
port = serial.Serial("/dev/serial0", baudrate=57600, timeout=3.0)
print("Starting UART test...")
port.reset_input_buffer()

#Open/create logs
log_handle=fresca_logging.start_logging(log_path)

try:
  while True:
    last_time = datetime.datetime.today().second
    
    packet = rx_packet(port)
    
    iter+=1
    
    if packet == False:
      print("Bad frame!!!")
      bad_crc+=1
    else:
      print("Good frame!!!")
      success+=1
        
      #Process the packet
      sensor_data=process_temp_packet(packet)
      
      #Make sure we don't print in the same second twice
      while ( ((last_time+log_interval)%60) != (datetime.datetime.today().second)):
        pass
      
      #Write data to log
      start_time=time.time()
      
      today = datetime.datetime.today()
      
      log_handle=fresca_data_log.LogTempSamples(log_handle,today,sensor_data)
      
      print("Logging took: "+str((time.time()-start_time)*1000))
      
  
except KeyboardInterrupt: # Ctrl+C pressed
  fresca_logging.end_logging(log_handle)
  port.close()
  print ("")
  print ("Iterations: "+str(iter))
  print ("Successful: "+str(success))
  print ("Failed: "+str(failure))
  print ("Bad CRC: "+str(bad_crc))
  print ("Success ratio: "+str((float(success)/float(iter))*100)+"%")
  print ("Exit!")
  quit()