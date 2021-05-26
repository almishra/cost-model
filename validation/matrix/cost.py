import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import StandardScaler
from sklearn.svm import SVR
from numpy import mean, std
import time
from sklearn.model_selection import KFold, cross_validate, cross_val_predict
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error, mean_squared_error, mean_absolute_percentage_error, r2_score
import pickle
import sys

test_size = sys.argv[1]

from warnings import simplefilter
simplefilter(action='ignore', category=FutureWarning)

dataset = pd.read_csv("data_matrix.csv")
dataset.columns = ((dataset.columns.str).replace("^ ","")).str.replace(" $","")
dataset = dataset.dropna()
col_names = ['collapse','simd','inner','Iter','VarDecl','refExpr','intLiteral','floatLiteral','mem_to','mem_from','add_sub_int','add_sub_double','mul_double','div_double','assign_int','assign_double']
scaler = StandardScaler()
features = scaler.fit_transform(dataset[col_names])
runtimes = dataset[['runtime']]

X_train, X_test, y_train, y_test = train_test_split(features, runtimes, test_size=int(test_size) / 100, random_state=0)
print(f'X_train = {X_train.shape}, X_test = {X_test.shape}, y_train = {y_train.shape}, y_test = {y_test.shape}')

model = SVR(kernel='poly')
start_time = time.time()
model.fit(X_train, y_train.values.ravel())
elapsed_time = (time.time() - start_time)

filename_model = 'model_' + test_size + '.sav'
pickle.dump(model, open(filename_model, 'wb'))
filename_scaler = 'scalar_' + test_size + '.sav'
pickle.dump(scaler, open(filename_scaler, 'wb'))

time_sec = elapsed_time % 60
time_min = (elapsed_time / 60) % 60
time_hr = (elapsed_time / 60) / 60
                                                                                
print(f'---- Training Time : {time_hr:.0f}:{time_min:.0f}:{time_sec:.0f} ----')

runtimes_pred = model.predict(X_test)
print(f'MAE = {mean_absolute_error(y_test, runtimes_pred)}')
print(f'RMSE = {np.sqrt(mean_squared_error(y_test, runtimes_pred))}')
print(f'MAPE = {mean_absolute_percentage_error(y_test, runtimes_pred)}')
print(f'R2 = {r2_score(y_test, runtimes_pred)}')


testset = pd.read_csv("test_case.csv")
test = scaler.transform(testset[col_names])
runtimes_test = testset[['runtime']]
runtimes_pred = model.predict(test)
print(runtimes_pred)
print(runtimes_test)
print(str(runtimes_pred))
