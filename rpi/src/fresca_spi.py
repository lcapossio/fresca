#!/usr/bin/python

#Script to fetch data from Arduino slave

import spidev
import time

#Constants
SPEED_HZ=int(0.1e6);
DELAY_USEC=int(500); #Delay in microseconds between transactions

#Commands/responses
START_CMD = int(0xA5);
FLUSH_CMD = int(0x1A);
IDLE_CMD  = int(0xAA);
TEMP_CMD  = int(0xAB);
HUM_CMD   = int(0xAC);
OK_CMD    = int(0x05);
NOK_CMD   = int(0xFA);


spi = spidev.SpiDev() # create spi object
spi.open(0, 0) # open spi port 0, device (CS) 0

spi.max_speed_hz=SPEED_HZ; #Set the SPI speed
spi.bits_per_word=8; #8 bits per word

payload_size=-1
iter=0
success=0
failure=0
bad_crc=0

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
    print "Temp "+str(i)+": "+str(temp[i])

try:

  while True:
    time.sleep(1) # sleep
    print "Trying to communicate with fresca Arduino"
    iter+=1
    
    start_time=time.time()
    
    #Try to communicate with Arduino until we get a response
    resp=[NOK_CMD]
    while resp[0] != OK_CMD:
        #Start SPI transaction
        resp = spi.xfer([START_CMD])

        #Send actual command (Get temperature)
        resp = spi.xfer([TEMP_CMD])
    
    #Next byte should be OK_CMD (to acknowledge command received OK)
    resp = spi.xfer([OK_CMD])
    if resp[0] != OK_CMD:
      print "***Abort communication, didn't get TEMP OK"
      failure+=1
      continue
    
    #First byte, payload size
    payload_size=spi.xfer([OK_CMD]);
    payload_size=payload_size[0];
    if payload_size == 0:
      print "***Abort communication, invalid payload"
      failure+=1
      continue
    
    print "Payload byte length: "+str(payload_size)
    
    #Receive data
    tx_buf = [OK_CMD] * payload_size;
    rx_buf = tx_buf;
    rx_buf = spi.xfer(tx_buf, SPEED_HZ, DELAY_USEC)

    #Receive CRC
    rx_crc = spi.xfer([OK_CMD], SPEED_HZ, DELAY_USEC)
    rx_crc = rx_crc[0]
    
    print "SPI took: "+str((time.time()-start_time)*1000)
    
    #Calculate crc value
    local_crc=0xff
    local_crc=calc_crc(payload_size,local_crc)
    for i in range(payload_size):
      local_crc=calc_crc(rx_buf[i],local_crc)

    #Check crc
    if local_crc != rx_crc:
      print "*Wrong crc, received: "+str(rx_crc)+", calculated: "+str(local_crc)
      bad_crc+=1
    else:
      print "Good frame!!! Rx: "+str(rx_crc)+", calculated: "+str(local_crc)
      success+=1
    
    for i in range(payload_size):
      print "Byte "+str(i)+": "+str(rx_buf[i])
      
    display_temp(rx_buf)
    
except KeyboardInterrupt: # Ctrl+C pressed, so...
  spi.close() # close the port before exit
  print ""
  print "Iterations: "+str(iter)
  print "Successful: "+str(success)
  print "Failed: "+str(failure)
  print "Bad CRC: "+str(bad_crc)
  print "Success ratio: "+str((float(success)/float(iter))*100)+"%"
  
  
  print "Exit!"