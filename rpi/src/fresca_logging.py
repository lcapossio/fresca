import os
import datetime
import tarfile
import csv
import multiprocessing

#Globals shared among this functions
csv_filename  = ''
csv_file_dir  = ''
do_compress   = True

#new_dir has to be the path to the CSV file directory
#Open a new fresca log file file
def start_logging(new_dir='',compress=True):
  global csv_file_dir,csv_filename,do_compress

  do_compress = compress
  
  if new_dir != '': #If new_dir is specified
    #Update directory
    csv_file_dir=new_dir
    
  curr_date=datetime.date.today()
  todays_file = csv_file_dir+'fresca_log_'+str(curr_date.year).rjust(4,'0')+str(curr_date.month).rjust(2,'0')+str(curr_date.day).rjust(2,'0')
  todays_file_csv = todays_file+'.csv'
  
  todays_file_tar = todays_file+'.tar.gz'
  if os.path.isfile(todays_file_tar): #If compressed file exists
    #Open the file an uncompress it
    tar = tarfile.open(todays_file_tar,'r:gz')
    tar.extractall(path=os.path.dirname(todays_file_tar))
    tar.close()
    #
    if not os.path.isfile(todays_file_csv):
      print('There was a problem uncompressing the CSV file')
      csv_file=None
    else:
      #Open for appending
      csv_file=open(todays_file_csv,'a')
      print('Recovered today\'s compressed log file for appending!')
  else:
    if os.path.isfile(todays_file_csv):
      #Open for appending
      csv_file=open(todays_file_csv,'a')
      print('Recovered today\'s un-compressed log file for appending!')
    else:
      #Create CSV file
      csv_file=open(todays_file_csv,'w')
      #Write header
      csvwriter = csv.writer(csv_file, delimiter=',')
      # 'epoch'     : epoch_time, #Time from 1970 Unix Epoch, used to sort/query documents easily
      # 'time'      : timestamp.isoformat(), #Timestamp of the sample in isoformat
      # 'Temp'      : sensor_data.getValue(sensor),
      # 'Hum'       : -1, #-1 means not supported
      # 'Idx'       : sensor,
      # 'Typ'       : 0,
      # 'HeatOn'    : 0, #0: OFF, 1: ON
      # 'CoolOn'    : 0  #0: OFF, 1: ON
      # csvwriter.writerow(['UNIX epoch','ISO time','Temperature','Humidity','Sensor index','Sensor type','HeaterOn','CoolerOn'])
      csvwriter.writerow(['Time','Temperature','Humidity','Sensor index','Sensor type','CoolerOn','HeaterOn'])
      print('New log file '+todays_file_csv+' created!')

  
  csv_filename = todays_file_csv #Update filename
  
  return csv_file
  
#Write log file into a compressed file (TAR+GZ)
def write_compress_csv_file(csv_filename):
  tar_filename = os.path.splitext(csv_filename)[0]+'.tar.gz'
  #print ('compressing to filename='+tar_filename)
  with tarfile.open(tar_filename, "w:gz") as tar:
    tar.add(csv_filename, arcname=os.path.basename(csv_filename), recursive=False)
  
  #Delete log csv file, leaving only the compressed tar file
  os.remove(csv_filename)
  
  return
  
def archive_csv_file(csv_filename):
  #Spawn a new process to compress the file as this may take a while and we need to keep logging
  p = multiprocessing.Process(target=write_compress_csv_file, args=(csv_filename,))
  p.start()
  
  return p
  # write_compress_csv_file(csv_filename) #Write-compress current file

def end_logging(csv_file):
  global csv_file_dir,csv_filename,do_compress
  
  #Write CSV file and close
  csv_file.close()
  
  #Archive the log (compress)
  if do_compress:
    archive_csv_file(csv_filename)

  return
  
#
def get_curr_csv_file(csv_file):
  global csv_file_dir,csv_filename
  
  curr_date=datetime.date.today()
  todays_file = csv_file_dir+'fresca_log_'+str(curr_date.year).rjust(4,'0')+str(curr_date.month).rjust(2,'0')+str(curr_date.day).rjust(2,'0')+'.csv'
  #print ('csv_file_dir='+csv_file_dir)
  #print ('csv_filename='+csv_filename)
  if (csv_filename != todays_file):
    #Create a new file, cause the day changed
    print('Creating new CSV file for a new day!')
    end_logging(csv_file)
    csv_file=start_logging()
    
  #Otherwise use same log file
  return csv_file