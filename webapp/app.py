#!/usr/bin/python

#fresca project by Leonardo Capossio

#sys.argv[1], server listen port
#sys.argv[2], Path to dump log files to


import psutil
from flask import Flask, render_template, request, url_for, send_from_directory, jsonify
from fresca_data import fresca_fetch
import time
import sys
import os

app = Flask(__name__)

# Constants
DEFAULT_PORT = 5000
DEFAULT_LOG_DIR = './logs/'

def get_args():
    port = DEFAULT_PORT
    log_dir = DEFAULT_LOG_DIR
    
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print(f"Invalid port: {sys.argv[1]}. Using default {DEFAULT_PORT}")
    
    if len(sys.argv) > 2:
        log_dir = sys.argv[2]
    
    if not os.path.isdir(log_dir):
        print(f"Warning: Log directory '{log_dir}' does not exist. Creating it.")
        os.makedirs(log_dir, exist_ok=True)
        
    return port, log_dir

listen_port, log_dir = get_args()
print(f"FRESCA Webapp starting on port {listen_port}")
print(f"Log files directory: {log_dir}")

#####################################
# Helper functions
#####################################

def is_fresca_link_running():
    """Check if fresca_uart_link.py is running in a platform-independent way."""
    for proc in psutil.process_iter(['cmdline']):
        try:
            cmdline = proc.info.get('cmdline')
            if cmdline and any('fresca_uart_link.py' in part for part in cmdline):
                return True
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return False

def assemble_filename(day, month, year):
    # Without the extension
    filename = f"fresca_log_{year:04d}{month:02d}{day:02d}"
    return os.path.join(log_dir, filename)

def chart_get(day, month, year, start_time=0, end_time=235959, sensor_idx=0, decimate_by=12, use_movavg=False):
    filename = assemble_filename(day, month, year)
    sens_data = fresca_fetch(filename, sensor_idx, start_time=start_time, end_time=end_time, decimate_samples=decimate_by, use_moving_avg=use_movavg)
    return sens_data

#####################################
# Routes
#####################################

@app.route('/')
def index():
    return render_template('index.html', fresca_link_running=is_fresca_link_running())

@app.route('/chart')
def chart():
    # Default to a specific date for now, or use today's date
    now = time.localtime()
    return render_template('chart.html', 
                          sensor=0, 
                          day=now.tm_mday, 
                          month=now.tm_mon, 
                          year=now.tm_year,
                          decimate_by=12, 
                          movavg_check=False,
                          start_hour=0, start_minute=0, start_second=0,
                          end_hour=23, end_minute=59, end_second=59,
                          fresca_link_running=is_fresca_link_running(),
                          draw_chart=True)

@app.route("/fresca_running", methods=['POST'])
def fresca_running():
    return jsonify(is_fresca_link_running())

@app.route("/chart_update", methods=['POST'])
def chart_update():
    try:
        decimate_by = int(request.form.get('decimate_select', 12))
        use_movavg = bool(int(request.form.get('movavg_check', 0)))
        sens = int(request.form.get('sens_select', 0))
        day = int(request.form.get('day_select', 1))
        month = int(request.form.get('month_select', 1))
        year = int(request.form.get('year_select', 2024))
        
        start_hour = request.form.get('hour_start_select', '00').rjust(2, '0')
        start_minute = request.form.get('minute_start_select', '00').rjust(2, '0')
        start_second = request.form.get('second_start_select', '00').rjust(2, '0')
        end_hour = request.form.get('hour_end_select', '23').rjust(2, '0')
        end_minute = request.form.get('minute_end_select', '59').rjust(2, '0')
        end_second = request.form.get('second_end_select', '59').rjust(2, '0')
        
        start_time = int(start_hour + start_minute + start_second)
        end_time = int(end_hour + end_minute + end_second)
        
        # Validation
        if end_time < start_time:
             # Basic correction: Swap them or set a minimal range
             start_time, end_time = end_time, start_time

        sens_data_header = [['Time', 'Temperature']]
        sens_data = chart_get(day, month, year, start_time=start_time, end_time=end_time, sensor_idx=sens, decimate_by=decimate_by, use_movavg=use_movavg)
        
        if sens_data is False or sens_data is None:
            return jsonify({'error': 'No data found'}), 404
        
        return jsonify(sens_data_header + sens_data)
        
    except Exception as e:
        return jsonify({'error': str(e)}), 400

@app.route('/robots.txt')
def static_from_root():
    return send_from_directory(app.static_folder, request.path[1:])

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=listen_port)
