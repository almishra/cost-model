import pandas as pd                                                             
from sklearn.preprocessing import StandardScaler                                
import pickle                                                                   

col_names = ['Iter','VarDecl','refExpr','intLiteral','floatLiteral','mem_to','mem_from','add_sub_int','add_sub_double','mul_int','mul_double','div_double','assign_int','assign_double','log_Iter','log_VarDecl','log_refExpr','log_intLiteral','log_floatLiteral','log_mem_to','log_mem_from','log_add_sub_int','log_add_sub_double','log_mul_int','log_mul_double','log_div_double','log_assign_int','log_assign_double']

model_file_cpu = open('CPU_model_50_50_50_30_relu.sav', 'rb')
model_cpu = pickle.load(model_file_cpu)
scaler_file_cpu = open('CPU_scaler_50_50_50_30_relu.sav', 'rb')
scaler_cpu = pickle.load(scaler_file_cpu)

model_file_gpu = open('GPU_model_50_50_50_30_relu.sav', 'rb')
model_gpu = pickle.load(model_file_gpu)
scaler_file_gpu = open('GPU_scaler_50_50_50_30_relu.sav', 'rb')
scaler_gpu = pickle.load(scaler_file_gpu)

testset = pd.read_csv('data_test_log.csv')                                          

test_gpu = scaler_gpu.transform(testset[col_names])                                     
runtimes_gpu = testset[['runtime_GPU']]                                            
runtimes_pred_gpu = model_gpu.predict(test_gpu)                                             

# Change memory transfer values to 0 for CPU
testset['mem_to'].values[:] = 0
testset['mem_from'].values[:] = 0
testset['log_mem_to'].values[:] = 0
testset['log_mem_from'].values[:] = 0
test_cpu = scaler_cpu.transform(testset[col_names])                                     
runtimes_cpu = testset[['runtime_CPU']]                                            
runtimes_pred_cpu = model_cpu.predict(test_cpu)                                             
                                                                                
runtimes_list_cpu = runtimes_cpu.values.tolist() 
runtimes_list_gpu = runtimes_gpu.values.tolist() 
for x in range(len(runtimes_cpu)):
  print(str(runtimes_list_cpu[x][0]) + ',' + str(runtimes_pred_cpu[x]) + ',' + str(runtimes_list_gpu[x][0]) + ',' + str(runtimes_pred_gpu[x]))
