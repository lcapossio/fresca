from flask import Flask, render_template, request, url_for, send_from_directory
from fresca_data import fresca_fetch
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

def assemble_filename(day,month,year):
  global log_dir
  
  #Without the extension
  return log_dir+'fresca_log_'+str(year).rjust(4,'0')+str(month).rjust(2,'0')+str(day).rjust(2,'0')

def chart_get(day,month,year,start_time=int(0),end_time=int(235959),sensor_idx=0,decimate_by=60,use_movavg=False):

  chart_start_time=time.time()
  
  filename=assemble_filename(day,month,year)
  
  sens_data=fresca_fetch(filename,sensor_idx,start_time=start_time,end_time=end_time,decimate_samples=decimate_by,use_moving_avg=use_movavg)
  
  print("Chart took: "+str((time.time()-chart_start_time)*1000))
  
  start_time_str=str(start_time).rjust(6,'0')
  end_time_str=str(end_time).rjust(6,'0')
  
  start_hour=int(start_time_str[0:2])
  start_minute=int(start_time_str[2:4])
  start_second=int(start_time_str[4:6])
  
  end_hour=int(end_time_str[0:2])
  end_minute=int(end_time_str[2:4])
  end_second=int(end_time_str[4:6])
  
  #Draw chart with data
  return render_template('chart.html',sens_data=sens_data, sensor=sensor_idx, day=day, month=month, year=year,
                          decimate_by=decimate_by,movavg_check=use_movavg,
                          start_hour=start_hour,start_minute=start_minute,start_second=start_second,
                          end_hour=end_hour,end_minute=end_minute,end_second=end_second,
                          draw_chart=(sens_data != False))


#####################################
#####################################
#####################################


@app.route('/')
def index():
  return render_template('index.html')

@app.route('/chart')
def chart():
  return chart_get(14,2,2018)

@app.route("/chart_update" , methods=['GET', 'POST'])
def chart_update():
  
    if request.method == 'POST':
    
      decimate_by = int(request.form.get('decimate_select'))
      if request.form.get('movavg_check'):
        use_movavg = True
      else:
        use_movavg = False
    
      if request.form['submit'] == 'Specific':
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
            
        return chart_get(day,month,year,start_time=start_time,end_time=end_time,sensor_idx=sens,decimate_by=decimate_by,use_movavg=use_movavg)
        
      elif request.form['submit'] == 'Latest':
        sens  = int(request.form.get('sens_select'))
        day=14
        month=2
        year=2018
        
        return chart_get(day,month,year,sensor_idx=sens,decimate_by=decimate_by,use_movavg=use_movavg)

    return chart_get(14,2,2018)
    
@app.route('/robots.txt')
def static_from_root():
    return send_from_directory(app.static_folder, request.path[1:])

if __name__ == '__main__':
  app.run(debug=True, host='0.0.0.0',port=listen_port)
