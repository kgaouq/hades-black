import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn import neighbors
from sklearn.metrics import accuracy_score

import ujson
import serial
import serial.tools.list_ports
from threading import Thread, Event, Lock
import queue
from http import client
from time import localtime, strftime
import tkinter as tk

from statistics import mean, stdev
from scipy.stats import kurtosis, skew

df = pd.read_csv("data\\data_15_15.csv", delimiter = ',')

# extract X, Y data
df_x = df.iloc[:, 0:-1]
df_y = df.iloc[:, 108]

X = df_x.to_numpy(dtype=float)
Y = df_y.to_numpy()

# Convert lables from string to int
##Y = np.zeros((len(Y_label), 1), dtype = int)
##Y[Y_label == "run"] = 0
##Y[Y_label == "sit"] = 1
##Y[Y_label == "stand"] = 2
##Y[Y_label == "walk"] = 3
##Y = np.concatenate(Y)

# train test split
X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.2, random_state = 42)

# knn model training
knn_clf = neighbors.KNeighborsClassifier(9, weights="distance")
knn_clf.fit(X, Y)

# knn model prediction
Y_pred = knn_clf.predict(X_test)
#print(Y_pred)
#print(Y_test)

#print(accuracy_score(Y_test, Y_pred))




def is_json(myjson):
    try:
        json_object = ujson.loads(myjson)
    except ValueError as e:
        return False
    return True


def dashboard(time, activity):
    print(f"mobile thread start")
    host = 'api.tago.io'
    url = 'https://api.tago.io/data'
    conn = client.HTTPSConnection(host)
    header ={
            'Content-Type': 'application/json',
            'Device-Token': '3438ba1c-0b10-4252-abbf-143aed8b368d',
            }
    time = strftime("%Y-%m-%d %H:%M:%S", localtime())
    payload = f'{{\"variable\":\"Activity\", \"value\":\"{activity}\", \"time\":\"{time}\"}}'
    print(payload)
    conn.request("POST", url, payload, header)
    response = conn.getresponse()
    print(('Response: ', response.status, response.reason))



def extract_features(df):
    features = np.array([])
    
    values = df.iloc[:, 0:].values
    
    for j in range(0, len(values[0, :])):
        acc = values[:, j]
        q1, q2, q3, q4, q5 = np.percentile(acc, [10, 25, 50, 75, 90])
        temp = np.array([mean(acc), stdev(acc), q1, q2, q3, q4, q5, q4 - q2, 
                max(acc) - min(acc), skew(acc), kurtosis(acc), np.sqrt(np.mean(acc**2))])
        
        features = np.concatenate((features, temp))
    return features


def feature_name():
    
    feature_name = []
    acc_axis = ["x0", "y0", "z0", "x1", "y1", "z1", "x2", "y2", "z2"]
    
    for name in acc_axis:
        temp = [name + "-mean", name + "-std", name + "-10th", name + "-25th",
                    name + "-50th", name + "-75th", name + "-90th", 
                    name + "-iqr", name + "-amp", name + "-skew", 
                    name + "-kurt", name + "-rms"]
        feature_name = feature_name + temp
    return feature_name



isConnected = False

def connect(port, queue, event):
    datas = {}
    count = 0
    data_count = 0
    serial_portname = port
    serial_port = serial.Serial(serial_portname, baudrate=115200, timeout=5.0)
    while event.is_set():
        input = serial_port.readline().decode(encoding='ascii',errors='ignore').rstrip()
        if len(input) == 0:
            print("Serial device timed out, no data received.")
        else:
            if is_json(input):
                json_obj = ujson.loads(input)
                data = {}
                time = 0
                for item in json_obj:
                    ID = item['node']
                    time = item['time']
                    datas.setdefault(f'x{ID}', []).append(item['X'])
                    datas.setdefault(f'y{ID}', []).append(item['Y'])
                    datas.setdefault(f'z{ID}', []).append(item['Z'])
                if(count >= 30):
                    for key in datas:
                        data[key] = datas[key][-15:]
                    df = pd.DataFrame(data)
                    features = [extract_features(df)]
                    df_test = pd.DataFrame(features, columns = feature_name())
                    pred = knn_clf.predict(df_test.iloc[:, 0:].to_numpy(dtype=float))
                    data_count += 1
                    queue.put_nowait(f'{pred[0]}')
                    print(f'{time}:{pred[0]}')
                    if data_count == 14 :
                        dash_thread = Thread(target = dashboard, args = (time, pred[0]))
                        dash_thread.start()
                        data_count = 0
                else:
                    print(count)
                    progress = count/30
                    queue.put_nowait(f'{progress:.2%}')
                count += 1
            else:
                print(input)


class GUI():
    def __init__(self):
        #self.threads = []
        self.queue = queue.Queue()
        self.event = Event()

        self.master = tk.Tk()
        self.text = tk.Label(self.master, text = "")
        self.select = tk.StringVar(self.master)
        self.ports_name = ['']
        self.select.set(self.ports_name[0]) # default value
        self.port_select = tk.OptionMenu(self.master, self.select, *self.ports_name)
        self.refresh_button = tk.Button(self.master, text='Refresh', command=self.refresh_port)
        self.start_button = tk.Button(self.master, text='Start', fg='green', command=self.start)



        self.master.geometry("400x400")
        self.master.title("GUI")
        self.text.config(width = 20, height = 5, font=("Helvetica", 25, "bold"))
        self.port_select.config(width=15, height = 1, font=("Helvetica", 15))
        self.refresh_button.config(width = 25, height = 2, font=("Helvetica", 15))
        self.start_button.config(width=25, height = 2, font=("Helvetica", 15))

        self.text.pack()
        self.port_select.pack()
        self.refresh_button.pack()
        self.start_button.pack()

        self.master.mainloop()

    def refresh_port(self):
        self.ports_name = []
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.ports_name.append(port.device)
        if not self.ports_name:
            self.ports_name = ['']
        self.select.set(self.ports_name[0])
        self.port_select['menu'].delete(0, 'end')
        for name in self.ports_name:
            self.port_select['menu'].add_command(label=name, command=tk._setit(self.select, name))

    def start(self):
        if self.select.get() == '':
            self.text.config(text = "no port!")
            return
        self.text.config(text = "")
        self.event.set()
        start_thread = Thread(target = connect, args = (self.select.get(), self.queue, self.event))
        self.start_button.config(text='End', fg='Red', command= lambda: self.end(start_thread))
        #self.threads.append(start_thread)
        start_thread.daemon = True
        start_thread.start()
        self.master.after(1, self.display)

    def end(self, thread):
        self.event.clear()
        self.start_button.config(text='Start', fg='green', command=self.start)
        self.text.config(text = "")
        thread.join()
        #self.threads = []
        with self.queue.mutex:
            self.queue.queue.clear()
    

    def display(self):
        if self.event.is_set():
            try:
                text = self.queue.get_nowait()
                self.text.config(text = text)
            except queue.Empty:
                pass
            self.master.after(100, self.display)
        else:
            return
        


gui = GUI()
