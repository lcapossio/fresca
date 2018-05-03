# fresca-webapp  

**fresca-webapp** gathers data from **fresca-link** log files and shows it in a web-like interface  

### Requirements

Python installed in your OS with **flask** for your python environment. On Linux do: `sudo pip install flask`. Currently only
tested for python 2.7  

### Usage

Start the webapp with: `sudo python app.py [http port] [log_dir]`  

In order to access the webapp you have to type in the IP address of the machine running it (Raspberry Pi or similar)
from a web browser. In Linux you can get the IP address by typing: `hostname -I`  

For example:
`sudo python app.py 80 ~/fresca-log/`  

*Note you can omit the `sudo` if you use a port higher than 1024*  