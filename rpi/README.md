# fresca-link  

This folder contains all **fresca-link** files, meant to be run in a host connected to the **fresca-controller**
(Arduino) via UART-link. Tests have been done with a Raspberry Pi Zero W.  

Currently **fresca-link** dumps one CSV file per day and compresses it in `tar.gz` when the day is finished, 
then it will start with a new CSV file.

### Description  

**fresca-controller** now outputs UART binary-data packets containing temperature, humidity and controller information periodically.
**fresca-link** is a program run on a host that listens to the UART connected to the **fresca-controller** and parses the packets
for data logging and/or displaying

### Usage  

Connect the host to the **fresca-controller** UART. You may need to use a level converter from 5v to 3.3v viceversa
depending on the host you are using (Raspberry Pi definitely requires this since it works with 3.3v)

Start the link with: ` sudo python fresca_uart_link.py [log_dir] [logging_interval_seconds] `
Look at the output to see if packets are coming from the controller. You can check the log directory
to see the CSV files being created


### CSV file format  

The header information reveals the data being written to the CSV file:  
`Time,Temperature,Humidity,Sensor index,Sensor type,CoolerOn,HeaterOn`  

* **Time** is written as a padded string in the form: `HHMMSS`.  
* **Humidity** is a value between 0 and 100%, except where not supported, in that case it is -1.  
* **Sensor type** currently has two values: `0: DS1820` and `1: DHT22`.  
* The rest of the values are natural integers or floating point values quite self-explanatory.