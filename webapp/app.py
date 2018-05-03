#!/usr/bin/python

#fresca project by Leonardo Capossio

#sys.argv[1], server listen port
#sys.argv[2], Path to dump log files to


from flask import Flask, render_template, request, url_for, send_from_directory,jsonify
from fresca_data import fresca_fetch
import subprocess
import time
import sys

app = Flask(__name__)

if len(sys.argv) != 3:
  print('Incorrect usage, arguments: [listen_port] [log_directory]')
  quit()

listen_port=int(sys.argv[1])

log_dir=str(sys.argv[2])
print('Log files dir: '+log_dir)

#####################################
#####################################
#####################################
#Helper functions

#Check if fresca-link is running
def is_fresca_link_running():
  output=str( subprocess.check_output('ps aux | grep R+ | grep fresca_uart_link.py -c', shell=True) ).split('\\n+')[0]
  print('is_fresca_link_running:'+str(int(output)))
  #Has to be greater than 1, cause the command we send is also counted
  if(int(output) > 1):
    return True
  return False

def assemble_filename(day,month,year):
  global log_dir
  
  #Without the extension
  return log_dir+'fresca_log_'+str(year).rjust(4,'0')+str(month).rjust(2,'0')+str(day).rjust(2,'0')

def chart_get(day,month,year,start_time=int(0),end_time=int(235959),sensor_idx=0,decimate_by=12,use_movavg=False):

  chart_start_time=time.time()
  
  filename=assemble_filename(day,month,year)
  
  sens_data=fresca_fetch(filename,sensor_idx,start_time=start_time,end_time=end_time,decimate_samples=decimate_by,use_moving_avg=use_movavg)
  
  print("Chart took: "+str((time.time()-chart_start_time)*1000))
  
  return sens_data

def chart_render(day,month,year,start_time=int(0),end_time=int(235959),sensor_idx=0,decimate_by=12,use_movavg=False):

  start_time_str=str(start_time).rjust(6,'0')
  end_time_str=str(end_time).rjust(6,'0')
  
  start_hour=int(start_time_str[0:2])
  start_minute=int(start_time_str[2:4])
  start_second=int(start_time_str[4:6])
  
  end_hour=int(end_time_str[0:2])
  end_minute=int(end_time_str[2:4])
  end_second=int(end_time_str[4:6])

  # return render_template('chart.html',sens_data=sens_data, sensor=sensor_idx, day=day, month=month, year=year,
  return render_template('chart.html',sensor=sensor_idx, day=day, month=month, year=year,
                          decimate_by=decimate_by,movavg_check=use_movavg,
                          start_hour=start_hour,start_minute=start_minute,start_second=start_second,
                          end_hour=end_hour,end_minute=end_minute,end_second=end_second,
                          fresca_link_running=is_fresca_link_running(),
                          # draw_chart=(sens_data != False))
                          draw_chart=True)

#####################################
#####################################
#####################################


@app.route('/')
def index():
  return render_template('index.html',fresca_link_running=is_fresca_link_running())

@app.route('/chart')
def chart():
  return chart_render(14,2,2018)

@app.route("/fresca_running" , methods=['POST'])
def fresca_running():
  return jsonify( is_fresca_link_running() )

@app.route("/chart_latest" , methods=['GET'])
def chart_lastest():
    sens  = int(request.form.get('sens_select'))
    day=14
    month=2
    year=2018

    sens_data = chart_get(day,month,year,sensor_idx=sens)
    if sens_data == False:
      return jsonify('0')
    else:
      return jsonify(sens_data)

@app.route("/chart_update" , methods=['POST'])
def chart_update():
  
  if request.method == 'POST':
  
    decimate_by = int(request.form.get('decimate_select'))
    if int(request.form.get('movavg_check')):
      use_movavg = True
    else:
      use_movavg = False

    sens  = int(request.form.get('sens_select'))
    day   = int(request.form.get('day_select'))
    month = int(request.form.get('month_select'))
    year  = int(request.form.get('year_select'))
    
    start_hour    = str(request.form.get('hour_start_select')).rjust(2,'0')
    start_minute  = str(request.form.get('minute_start_select')).rjust(2,'0')
    start_second  = str(request.form.get('second_start_select')).rjust(2,'0')
    end_hour      = str(request.form.get('hour_end_select')).rjust(2,'0')
    end_minute    = str(request.form.get('minute_end_select')).rjust(2,'0')
    end_second    = str(request.form.get('second_end_select')).rjust(2,'0')
    
    start_time = int(start_hour+start_minute+start_second)
    end_time   = int(end_hour+end_minute+end_second)
    
    #Minimum 10 mins
    if (end_time-start_time < 1000) or (end_time < start_time):
      end_time = 10+int(start_minute)
      #Check corner case
      if (end_time > 59):
        end_time=(int(start_hour)+1)*10000+(end_time-60)*100
      else:
        end_time=start_time+1000
        
    sens_data_header=[['Time','Temperature']]
    sens_data = chart_get(day,month,year,start_time=start_time,end_time=end_time,sensor_idx=sens,decimate_by=decimate_by,use_movavg=use_movavg)
    
    if sens_data == False:
      return jsonify('0'), 404 #Return 404
    else:
      send_data = sens_data_header+sens_data
      return jsonify(send_data)
    
  return jsonify('0'), 404 #Return 404
    
@app.route('/robots.txt')
def static_from_root():
    return send_from_directory(app.static_folder, request.path[1:])

if __name__ == '__main__':
  app.run(debug=True, host='0.0.0.0',port=listen_port)
