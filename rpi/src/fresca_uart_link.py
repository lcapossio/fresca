#!/usr/bin/python

import time
import serial

#Commands/responses
START_CMD = int(0xA5);
FLUSH_CMD = int(0x1A);
IDLE_CMD  = int(0xAA);
TEMP_CMD  = int(0xAB);
HUM_CMD   = int(0xAC);
OK_CMD    = int(0x05);
NOK_CMD   = int(0xFA);

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
  output=[float(0)]*(len(data)/2);
  for i in range(len(data)/2):
    temp = int( (data[(i*2)+1])<<8 | data[i*2] );
    if (temp & 0x8000): #Test sign-bit
      #Sign extend
      temp = (int(-1) & (~int(0xffff))) | temp
    
    output[i] = float(temp)/(2**4) #Cast to float and convert from fixed-point
  return output;
  
def display_temp(byte_buf):

  temp=parse_temp(byte_buf)
  
  for i in range(len(temp)):
    print("Temp "+str(i)+": "+str(temp[i]))


payload_size=-1
iter=0
success=0
failure=0
bad_crc=0


port = serial.Serial("/dev/serial0", baudrate=57600, timeout=3.0)
print("Starting UART test...")
port.reset_input_buffer()

#Reading serial port in binary mode (no text)
#ord() call is really important to turn unicode strings into bytes (I don't know why they made it so difficult)
try:
  while True:
    recv = ord(port.read(1))
    if recv == START_CMD:
      print("Got start of packet")
      iter+=1
      start_time=time.time()
      recv = ord(port.read(1))
      if recv == TEMP_CMD:
        #Temperature command
        #First byte, payload size
        payload_size=ord(port.read(1));
        if payload_size == 0:
          print("***Abort communication, invalid payload")
          failure+=1
          continue
        
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

        #Checksum
        if local_crc != rx_crc:
          print("*Wrong checksum, received: "+str(rx_crc)+", calculated: "+str(local_crc))
          bad_crc+=1
        else:
          print("Good frame!!! Checksum rx: "+str(rx_crc)+", calculated: "+str(local_crc))
          success+=1
        
        # for i in range(payload_size):
          # print("Byte "+str(i)+": "+str(rx_buf[i]))
          
        display_temp(rx_buf)
        
  
except KeyboardInterrupt: # Ctrl+C pressed
  port.close()
  print ("")
  print ("Iterations: "+str(iter))
  print ("Successful: "+str(success))
  print ("Failed: "+str(failure))
  print ("Bad CRC: "+str(bad_crc))
  print ("Success ratio: "+str((float(success)/float(iter))*100)+"%")
  print ("Exit!")
  quit()