from flask import Flask, jsonify, render_template, request
import threading, queue
import serial
import time
import numpy as np
from datetime import datetime


PORTS = ["/dev/cu.usbserial-14220", "/dev/cu.usbserial-14240",
         "/dev/cu.usbserial-14120", "/dev/cu.usbserial-14140"]
# todo need we pass it somewhere into save threading (though it seems that we won't 
# split the whole thing up)
serialport = None

app = Flask(__name__, template_folder='./templates', static_folder='./static')
# TODO: do we need to  pass here a maxsize? 
q = queue.Queue()

def open_port() -> None:
    global serialport
    if not serialport:
        # open serial port
        for port in PORTS:
            try:
                serialport = serial.Serial(port, 9600)
                break
            except Exception:
                continue
        if (not serialport):
            raise ValueError("Could not open the port")

    time.sleep(3)
    print("Port is openned")


def data_collection():
    while True:
        #read is blocking so waits till next packet of data is sent
        data = serialport.readline().decode("utf-8").strip()
        # :-1 because there is ; at the end of string which produces '' as the last 4 element
        try:
            x, y, z = map(float, data.split(";")[:-1])
        except Exception as err:
            # print(err) # TODO: logging here
            continue
        # print(x, y, z)
        #put data in queue
        q.put(y)


@app.route("/update", methods = ['GET'])
def update_chart():
    data = []
    while not q.empty():
        data.append(q.get())

    return jsonify(results = [np.array(data).mean(), datetime.now().strftime("%H:%M")])


@app.route("/")
def index():
    return render_template('index.html')

if __name__ == '__main__':
    open_port()
    x = threading.Thread(target=data_collection)
    x.start()
    app.run(host='0.0.0.0', port=5000)
