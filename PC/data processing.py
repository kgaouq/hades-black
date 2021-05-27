import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import math
from scipy import signal
from statistics import mean, stdev
from scipy.stats import kurtosis, skew

def isfloat(value):
    try:
        value.astype(float)
        return True
    except ValueError:
        return False
    
def find_error_data(df):
    idx = df.index
    a = list(df['node'].astype(int))
    diff = np.array([a[n]-a[n-1] for n in range(1,len(a))])
    print(idx[np.where((diff != 1) & (diff != -2))])

def calculate_acc_mag(acc):
    return math.sqrt(acc[0] ** 2 + acc[1] ** 2 + acc[2] ** 2)

def plot_data(df):
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1)
    time = np.array(list(range(len(df))))
    acc_0_x = df['x0'].values
    acc_0_y = df['y0'].values
    acc_0_z = df['z0'].values
    acc_1_x = df['x1'].values
    acc_1_y = df['y1'].values
    acc_1_z = df['z1'].values
    acc_2_x = df['x2'].values
    acc_2_y = df['y2'].values
    acc_2_z = df['z2'].values
    # print(time)
    # print(acc_0_x)
    ax1.plot(time, acc_0_x)
    ax1.plot(time, acc_0_y)
    ax1.plot(time, acc_0_z)
    ax2.plot(time, acc_1_x)
    ax2.plot(time, acc_1_y)
    ax2.plot(time, acc_1_z)
    ax3.plot(time, acc_2_x)
    ax3.plot(time, acc_2_y)
    ax3.plot(time, acc_2_z)
    plt.show()

def filter_data(df):
    fs = 5
    fc = 2  # Cut-off frequency of the filter
    w = fc / (fs / 2) # Normalize the frequency
    b, a = signal.butter(5, w, 'low')
    
    df['x0'] = signal.filtfilt(b, a, df['x0'].values)
    df['y0'] = signal.filtfilt(b, a, df['y0'].values)
    df['z0'] = signal.filtfilt(b, a, df['z0'].values)
    df['x1'] = signal.filtfilt(b, a, df['x1'].values)
    df['y1'] = signal.filtfilt(b, a, df['y1'].values)
    df['z1'] = signal.filtfilt(b, a, df['z1'].values)
    df['x2'] = signal.filtfilt(b, a, df['x2'].values)
    df['y2'] = signal.filtfilt(b, a, df['y2'].values)
    df['z2'] = signal.filtfilt(b, a, df['z2'].values)
    
    return df 

def extract_features(df):
    features = np.array([])
    
    values = df.iloc[:, 0:-1].values
    
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
    
############################### load data ####################################
file_list = ["run.txt", "run2.txt", "sit.txt", "sit2.txt", "stand.txt", 
             "stand2.txt", "walk.txt", "walk2.txt"]
file_list = ["data\\" + file for file in file_list]
data_type = ["run", "run2", "sit", "sit2", "stand", "stand2", "walk", "walk2"]

df_collection = {}

for i in range(len(file_list)):
    
    print(file_list[i])
    df = pd.read_csv(file_list[i], delimiter = ' ', names = ["node", "x", "y", "z", "Time"])
    
    # Remove invalid data (invalid node id)
    df = df[(df['node'] == '0') | (df['node'] == '1') | (df['node'] == '2')]
    df = df.iloc[list(range(math.floor(len(df) / 3) * 3)), :]
    
    # reorgnize data frame to synchronize data from different nodes
    df_0 = df[df['node'] == '0']
    df_1 = df[df['node'] == '1']
    df_2 = df[df['node'] == '2']
    
    data = {'x0': list(df_0['x']), 'y0': list(df_0['y']), 'z0': list(df_0['z']), 
            'x1': list(df_1['x']), 'y1': list(df_1['y']), 'z1': list(df_1['z']), 
            'x2': list(df_2['x']), 'y2': list(df_2['y']), 'z2': list(df_2['z'])}
    df_update = pd.DataFrame(data)
    
    # remove invalid data (invalid acc values)
    keep_idx = []
    for j in range(len(df_update)):
        temp = df_update.iloc[j].to_numpy()
        if isfloat(temp):
            keep_idx.append(j)
    
    df_update = df_update.iloc[keep_idx, :]
    df_update.reset_index(drop = True, inplace = True)
    
    # add motion type to each sample
    df_update["type"] = data_type[i]
    
    # remove invalid data (acc value all 0s)
    rm_idx = []
    
    for j in range(len(df_update)):
        temp = df_update.iloc[j].to_numpy()
        acc_0 = temp[0:3].astype(float)
        acc_1 = temp[3:6].astype(float)
        acc_2 = temp[6:9].astype(float)
        
        if any(mag == 0 for mag in [calculate_acc_mag(acc_0), calculate_acc_mag(acc_1), calculate_acc_mag(acc_2)]):
            rm_idx.append(j)
            
    df_update = df_update.drop(labels=rm_idx, axis=0)
    df_update.reset_index(drop = True, inplace = True)
    df_update.iloc[:, 0:-1] = df_update.iloc[:,0:-1].astype(float)
    df_collection[data_type[i]] = df_update
    
############################### Process data ###################################
print("\n\n\n")

for key in df_collection:
    print(key)
    
    # filter data
    # df_collection[key] = filter_data(df_collection[key])
    
    df = df_collection[key]
    
    # feature extraction
    features = []
    
    for i in range(0, len(df) - 15, 15):
        window = np.array(range(i, i + 15))
        df_window = df.iloc[window, :]
        features_window = extract_features(df_window)
        features.append(features_window)
    
    df = pd.DataFrame(features, columns = feature_name())
    
    if key == "run" or key == "run2":
        df['type'] = "run"
    elif key == "sit" or key == "sit2":
        df['type'] = "sit"
    elif key == "stand" or key == "stand2":
        df["type"] = "stand"
    elif key == "walk" or key == "walk2":
        df["type"] = "walk"
    
    if key == "run":
        df_final = df
    else:
        df_final = df_final.append(df, ignore_index = True)
  
df_final.dropna(inplace = True)
df_final.to_csv("data\\data_15_15.csv", index = False)
    
    
