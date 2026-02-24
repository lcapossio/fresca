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
  
  # Iterating over the file object directly is memory-efficient
  try:
    # Get header info from first row
    header = next(csv_file).strip().split(',')
    
    time_col = header.index('Time') if 'Time' in header else 0
    sens_idx_col = header.index('Sensor index') if 'Sensor index' in header else 3
    temp_col = header.index('Temperature') if 'Temperature' in header else 1
      
    #Loop through rows in reader
    #Grab the temperature from the correct sensor index
    
    decimate = 0
    total_data_points = 0
    first_sample = True
    movaverage_len = max(decimate_samples, 8)
    
    # We need to skip to the first sample that matches sensor_idx
    # The files are written with all sensors sequentially for each time step
    # So we can calculate how many lines to skip or just check if row[sens_idx_col] == str(sensor_idx)
    
    for row_str in csv_file:
      row = row_str.strip().split(',')
      if not row or len(row) <= max(time_col, sens_idx_col, temp_col):
        continue
      
      try:
        current_sens_idx = int(row[sens_idx_col])
      except (ValueError, IndexError):
        continue

      if current_sens_idx != sensor_idx:
        continue

      time_str = row[time_col]
      try:
        time_int = int(time_str)
      except ValueError:
        continue

      if time_int < start_time:
        continue
      
      if time_int > end_time:
        break
      
      try:
        sample = float(row[temp_col])
      except ValueError:
        continue

      if use_moving_avg:
        if first_sample:
          samples = [sample] * movaverage_len
          cumsum = sample * movaverage_len
          first_sample = False
        else:
          cumsum += sample - samples.pop(0)
          samples.append(sample)
        
        if decimate == decimate_samples - 1:
          result = cumsum / movaverage_len
          decimate = 0
          sens_data.append([time_str[0:2]+':'+time_str[2:4]+':'+time_str[4:6], result])
          total_data_points += 1
        else:
          decimate += 1
      else:
        # For decimation without moving average, we only take every Nth sample
        if decimate == 0:
          sens_data.append([time_str[0:2]+':'+time_str[2:4]+':'+time_str[4:6], sample])
          total_data_points += 1
        
        decimate = (decimate + 1) % decimate_samples

      if total_data_points >= max_datapoints:
        print('Fetched '+str(total_data_points)+' data points.')
        break

  except Exception as e:
    print('ERROR: Caught \"'+str(e)+'\" while reading '+csv_filename)
    if not sens_data:
      sens_data = False
  
  finally:
    csv_file.close()
    if remove_after_reading and os.path.isfile(csv_filename):
      os.remove(csv_filename)
  
  return sens_data
