#Leonardo Capossio for fresca project
#Log sensor data into a CSV file

import fresca_logging
import csv

#timestamp has to be of 'datetime' format
def LogTempSamples(csv_file,timestamp,sensor_data):
  
  csv_file=fresca_logging.get_curr_csv_file(csv_file)
  csvwriter = csv.writer(csv_file, delimiter=',')
  
  for sensor in range(sensor_data.getNumSensors()):
    # documents_cache.append(  {
                  # 'epoch'     : epoch_time, #Time from 1970 Unix Epoch, used to sort/query documents easily
                  # 'time'      : timestamp.isoformat(), #Timestamp of the sample in isoformat
                  # 'Temp'      : sensor_data.getValue(sensor),
                  # 'Hum'       : -1, #-1 means not supported
                  # 'Idx'       : sensor,
                  # 'Typ'       : 0,
                  # 'CoolOn'    : 0, #0: OFF, 1: ON
                  # 'HeatOn'    : 0  #0: OFF, 1: ON
                # }
                # )
    #Write to CSV
    # csvwriter.writerow([
                        # epoch_time,
                        # timestamp.isoformat(),
                        # "{:03.3f}".format(sensor_data.getValue(sensor)),
                        # "{:03d}".format(-1),
                        # "{:02d}".format(sensor),
                        # 0,0,0
                        # ])
                        
    csvwriter.writerow([
                        str(timestamp.hour).rjust(2,'0')+str(timestamp.minute).rjust(2,'0')+str(timestamp.second).rjust(2,'0'),
                        "{:.2f}".format(sensor_data.getValue(sensor)),
                        str(-1),
                        str(sensor),
                        0,0,0
                        ])
  
  return csv_file