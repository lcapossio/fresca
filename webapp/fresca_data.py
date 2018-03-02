#fresca project by Leonardo Capossio

import os
import csv
import tarfile
import itertools

#CSV Header: 'Time','Temperature','Humidity','Sensor index','Sensor type','CoolerOn','HeaterOn'

def fresca_fetch(filename,sensor_idx,start_time=int(0),end_time=int(235959),num_sensors=8,decimate_samples=60,use_moving_avg=False,max_datapoints=1500):

  tar_filename = filename+'.tar.gz'
  csv_filename = filename+'.csv'
  remove_after_reading=False
  
  print(start_time)
  print(end_time)
  
  if os.path.isfile(tar_filename): #If compressed file exists
    #Open the file an uncompress it
    print('Extracting '+tar_filename)
    tar = tarfile.open(tar_filename,'r:gz')
    tar.extractall(path=os.path.dirname(tar_filename))
    tar.close()
    if not os.path.isfile(csv_filename):
      print('There was a problem uncompressing '+tar_filename)
      csv_file=None
      return False
    else:
      #Open for reading
      csv_file=open(csv_filename,'r')
      print('Opened '+csv_filename+' log file for reading!')
      remove_after_reading=True
    
  elif os.path.isfile(csv_filename):
    #Open for reading
    csv_file=open(csv_filename,'r')
    print('Opened '+csv_filename+' log file for reading!')
    
  else:
    #No files!
    return False
  
  #Buffer all the lines, that is the fastest way! (files need to be small though, relative to available memory) (csvreader sucks)
  lines=csv_file.readlines()
    
  # ######################
  #Start reading the file
  # ######################
  
  sens_data = []
  first_sample=True
  movaverage_len=max(decimate_samples,8)
  decimate=0
  total_data_points=0
  
  try:
    #Get header info from first row
    colnum = 0
    row = lines[0].split(',')
    for col in row:
    
      if row[colnum] == 'Time':
        time_col = colnum
      if row[colnum] == 'Sensor index':
        sens_idx_col = colnum
      if row[colnum] == 'Temperature':
        temp_col = colnum
        
      colnum += 1
      
    #Loop through rows in reader making jumps every 'num_sensors'
    #Grab the temperature from the correct sensor index
    if use_moving_avg:
      step=num_sensors
    else:
      step=num_sensors*decimate_samples
      
    for line in itertools.islice(lines, sensor_idx+1, None, step):
      row = line.split(',') #Split csv fields into separate strings
      time_str=str(row[time_col])
      time_int=int(time_str)
      sample=float(row[temp_col])
      
      if (time_int >= start_time):
      
        if (end_time < time_int): #Stop if end time has passed (samples are assumed sorted in ascending time)
          break
          
        if use_moving_avg:
        
          if first_sample:
            samples=[sample]*movaverage_len #Initialize the moving average window
            cumsum=sample*movaverage_len    #Initialize the cumulative sum
            result=sample                   #
            first_sample=False
          else:
            cumsum+=sample-samples.pop(0) #also remove old sample
            samples.append(sample) #insert new sample
        
          if decimate == decimate_samples-1:
            result=cumsum/movaverage_len
            decimate=0
            sens_data.append( [ time_str[0:2]+':'+time_str[2:4]+':'+time_str[4:6],
                               result ] )
            total_data_points+=1
          else:
            decimate+=1
            
        else:
        
          result=sample
          total_data_points+=1
          sens_data.append( [ time_str[0:2]+':'+time_str[2:4]+':'+time_str[4:6],
                             result ] )
        if total_data_points == max_datapoints:
          #Finish here so we won't hurt performance
          print('Fetched only '+str(max_datapoints)+' to save performance')
          break

  except Exception as e:
    #Probably some read error cause the file is incomplete or broken
    if len(sens_data) > 1:
      #Cut the last value from the list and return whatever we got so far
      del sens_data[-1]
    else:
      #Something very wrong with the file or the parsing
      print('ERROR: Caught \"'+str(e)+'\" while reading '+csv_filename+'. Returning no data')
      sens_data = False
  
  #
  if remove_after_reading:
    os.remove(csv_filename)
  
  csv_file.close()
  
  return sens_data
