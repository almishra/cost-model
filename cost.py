import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import StandardScaler
from sklearn.svm import SVR
from numpy import mean, std
import time
from sklearn.model_selection import KFold, cross_validate, cross_val_predict

from warnings import simplefilter
simplefilter(action='ignore', category=FutureWarning)

def plot_regression_results(ax, y_true, y_pred, title, scores, elapsed_time):
  """Scatter plot of the predicted vs true targets."""
  ax.plot([y_true.min(), y_true.max()], 
          [y_true.min(), y_true.max()], 
          '--r', linewidth=2)
  ax.scatter(y_true, y_pred, alpha=0.2)

  ax.spines['top'].set_visible(False)
  ax.spines['right'].set_visible(False)
  ax.get_xaxis().tick_bottom()
  ax.get_yaxis().tick_left()
  ax.spines['left'].set_position(('outward', 10))
  ax.spines['bottom'].set_position(('outward', 10))
  ax.set_xlim([y_true.min(), y_true.max()])
  ax.set_ylim([y_true.min(), y_true.max()])
  ax.set_xlabel('Measured')
  ax.set_ylabel('Predicted')
  extra = plt.Rectangle((0, 0), 0, 0, fc="w", fill=False, 
                        edgecolor='none', linewidth=0)
  ax.legend([extra], [scores], loc='upper left')
  ax.set_title(title)

dataset = pd.read_csv("gpu_data_seawulf.csv")
#dataset = pd.read_csv("gpu_data_small.csv")
dataset.columns = ((dataset.columns.str).replace("^ ","")).str.replace(" $","")
dataset = dataset.dropna()
col_names = ['collapse','Iter','VarDecl','refExpr','intLiteral','floatLiteral','mem_to','mem_from','add_sub_int','add_sub_double','mul_double','div_double','assign_int','assign_double', 'runtime']
data = StandardScaler().fit_transform(dataset[col_names])
features = data[:,0:14:1]
runtimes = data[:,14]

fig, axs = plt.subplots()

cv = KFold(n_splits=10, random_state=1, shuffle=True)
model = SVR(kernel='poly')
start_time = time.time()
scoring = [ 'neg_mean_absolute_error', 
            'neg_root_mean_squared_error',
            'neg_mean_absolute_percentage_error', 
            'r2']

score = cross_validate(model, features, runtimes.ravel(), scoring=scoring, cv=cv, n_jobs=10, return_train_score=True, return_estimator=True) 
elapsed_time = (time.time() - start_time)

runtime_pred = cross_val_predict(model, features, runtimes.ravel(), n_jobs=10, verbose=0)

time_sec = elapsed_time % 60
time_min = (elapsed_time / 60) % 60
time_hr = (elapsed_time / 60) / 60

plot_regression_results(axs, runtimes.ravel(), runtime_pred, (f'---- Evaluation in {time_hr:.0f}:{time_min:.0f}:{time_sec:.0f} ----'),
                        (r'$MAE={:.2f} \pm {:.2f}$' + '\n' + r'$RMSE={:.2f} \pm {:.2f}$' + '\n' + r'$MAPE={:.2f} \pm {:.2f}$' + '\n' + r'$R^2={:.2f} \pm {:.2f}$')
                            .format(-mean(score['test_neg_mean_absolute_error']), std(score['test_neg_mean_absolute_error']),
                                    -mean(score['test_neg_root_mean_squared_error']), std(score['test_neg_root_mean_squared_error']),
                                    -mean(score['test_neg_mean_absolute_percentage_error']), std(score['test_neg_mean_absolute_percentage_error']),
                                    mean(score['test_r2']), std(score['test_r2'])),
                            elapsed_time)
plt.tight_layout()
plt.savefig('plot.png')
                                                                                
print(f'---- {time_hr:.0f}:{time_min:.0f}:{time_sec:.0f} ----') 

print(score.keys())
print('neg_mean_absolute_error:  %.3f (%.3f)' % (mean(score['test_neg_mean_absolute_error']), std(score['test_neg_mean_absolute_error'])))
print('neg_root_mean_squared_error:  %.3f (%.3f)' % (mean(score['test_neg_root_mean_squared_error']), std(score['test_neg_root_mean_squared_error'])))
print('neg_mean_absolute_percentage_error:  %.3f (%.3f)' % (mean(score['test_neg_mean_absolute_percentage_error']), std(score['test_neg_mean_absolute_percentage_error'])))
print('r2:  %.3f (%.3f)' % (mean(score['test_r2']), std(score['test_r2'])))
