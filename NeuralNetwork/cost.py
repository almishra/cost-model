import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import StandardScaler
from sklearn.svm import SVR
from numpy import mean, std
import time
from sklearn.metrics import mean_absolute_error, mean_squared_error, mean_absolute_percentage_error, r2_score
from sklearn.neural_network import MLPRegressor
from sklearn.model_selection import train_test_split
import pickle
import sys

file = sys.argv[1]

from warnings import simplefilter
simplefilter(action='ignore', category=FutureWarning)

dataset = pd.read_csv(file)
dataset.columns = ((dataset.columns.str).replace("^ ","")).str.replace(" $","")
dataset = dataset.dropna()
col_names = ['Iter','VarDecl','refExpr','intLiteral','floatLiteral','mem_to','mem_from','add_sub_int','add_sub_double','mul_int','mul_double','div_double','assign_int','assign_double','log_Iter','log_VarDecl','log_refExpr','log_intLiteral','log_floatLiteral','log_mem_to','log_mem_from','log_add_sub_int','log_add_sub_double','log_mul_int','log_mul_double','log_div_double','log_assign_int','log_assign_double']
scaler = StandardScaler()

features = scaler.fit_transform(dataset[col_names])
runtimes = dataset[['runtime']]

print("Total Data point,Runtime Range,Iteration,Rate,Momentum,Hidden_layers,Activation Funtion,Maximum deviation,Outlier_5,Outlier_5(%),Outlier_10,Outlier_10(%),Outlier_15,Outlier_15(%),SMAPE,RMSE,MAPE")
for layer1 in [50]:
    for layer2 in [50]:
        for layer3 in [50]:
            for layer4 in [30]:
                for act in ['relu']: #, 'logistic', 'tanh']:
                    model = MLPRegressor(max_iter=1000, hidden_layer_sizes=(layer1,layer2,layer3,layer4), momentum=0.85, learning_rate_init=0.001, activation=act)
                    #start_time = time.time()
                    model.fit(features, runtimes.values.ravel())
                    #elapsed_time = (time.time() - start_time)
                    #time_sec = elapsed_time % 60
                    #time_min = (elapsed_time / 60) % 60
                    #time_hr = (elapsed_time / 60) / 60
                    #print(f'---- Training Time for {layer1}_{layer2}_{layer3}_{layer4}_{act} : {time_hr:.0f}:{time_min:.0f}:{time_sec:.0f} ----')
    
                    #filename_model = 'GPU_model_' + str(layer1) + '_' + str(layer2) + '_' + str(layer3) + '_' + str(layer4) + '_' + act + '.sav'
                    filename_model = 'CPU_model_' + str(layer1) + '_' + str(layer2) + '_' + str(layer3) + '_' + str(layer4) + '_' + act + '.sav'
                    #filename_model = 'Seawulf_model_' + str(layer1) + '_' + str(layer2) + '_' + str(layer3) + '_' + str(layer4) + '_' + act + '.sav'
                    pickle.dump(model, open(filename_model, 'wb'))
                    #filename_scaler= 'GPU_scaler_' + str(layer1) + '_' + str(layer2) + '_' + str(layer3) + '_' + str(layer4) + '_' + act + '.sav'
                    filename_scaler= 'CPU_scaler_' + str(layer1) + '_' + str(layer2) + '_' + str(layer3) + '_' + str(layer4) + '_' + act + '.sav'
                    #filename_scaler= 'Seawulf_scaler_' + str(layer1) + '_' + str(layer2) + '_' + str(layer3) + '_' + str(layer4) + '_' + act + '.sav'
                    pickle.dump(scaler, open(filename_scaler, 'wb'))

                    runtimes_list = runtimes.values.tolist()
                    runtimes_pred = model.predict(features)

                    SMAPE = 0
                    max_dev = 0
                    min = 10000
                    max = 0
                    min_orig = 10000
                    max_orig = 0
                    num_dev_5 = 0
                    num_dev_10 = 0
                    num_dev_15 = 0
                    for x in range(len(runtimes)):
                        SMAPE = SMAPE + abs(runtimes_list[x][0] - runtimes_pred[x]) / ((runtimes_list[x][0] + runtimes_pred[x]) / 2)
                        dev = abs(runtimes_list[x][0] - runtimes_pred[x])
                        dev_per = dev / runtimes_list[x][0] * 100
                        if dev_per > 5.0:
                            num_dev_5 = num_dev_5 + 1
                        if dev_per > 10.0:
                            num_dev_10 = num_dev_10 + 1
                        if dev_per > 15.0:
                            num_dev_15 = num_dev_15 + 1
                        max_dev = dev if dev > max_dev else max_dev
                        min = runtimes_pred[x] if runtimes_pred[x] < min else min
                        max = runtimes_pred[x] if runtimes_pred[x] > max else max
                        min_orig = runtimes_list[x][0] if runtimes_list[x][0] < min_orig else min_orig
                        max_orig = runtimes_list[x][0] if runtimes_list[x][0] > max_orig else max_orig
                    SMAPE = SMAPE / len(runtimes)
                    out_5 = num_dev_5 / len(runtimes) * 100
                    out_10 = num_dev_10 / len(runtimes) * 100
                    out_15 = num_dev_15 / len(runtimes) * 100
                    RMSE = np.sqrt(mean_squared_error(runtimes, runtimes_pred))
                    MAPE = mean_absolute_percentage_error(runtimes, runtimes_pred)
                    print(f'{len(runtimes)},{min_orig:.3f} - {max_orig:.3f},1000,0.001,0.85,"({layer1},{layer2},{layer3},{layer4})",{act},{max_dev:.4f},{num_dev_5},{out_5:.4f}%,{num_dev_10},{out_10:.4f}%,{num_dev_15},{out_15:.4f}%,{SMAPE:.4f},{RMSE:.4f},{MAPE:.4f}')
