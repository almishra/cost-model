import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
import pandas as pd
from sklearn.model_selection import train_test_split
from torch.utils.data.sampler import SubsetRandomSampler
from torch.utils.data import Dataset, DataLoader
from sklearn.preprocessing import MinMaxScaler, StandardScaler, MaxAbsScaler
from torchsummary import summary
import numpy as np
import copy
import random
from collections import OrderedDict
from sklearn.metrics import mean_absolute_error, mean_squared_error, mean_absolute_percentage_error, r2_score

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

class PrepareData(Dataset):
    def __init__(self, X, y, train=True, scaler_obj=None):
        self.sc1 = StandardScaler()
        if not torch.is_tensor(X):
            if train:
                X = self.sc1.fit_transform(X)
                self.X = torch.from_numpy(X)
            else:
                if scaler_obj is not None:
                    X = scaler_obj.transform(X)
                    self.X = torch.from_numpy(X)
                else:
                    print('include scaler object from training')
        if not torch.is_tensor(y):
            y = y.to_numpy()
            y = np.reshape(y, (-1,1))
            if train:
                print(type(y.dtype), y.dtype)
                self.y = torch.from_numpy(y)
            else:
                self.y = torch.from_numpy(y)

    def __len__(self):
        return len(self.X)

    def __getitem__(self, idx):
        return self.X[idx], self.y[idx]
    
    def return_scaler_obj(self):
        return self.sc1


class KernelRunModel(torch.nn.Module):
    def __init__(self, ip_features, num_hidden, op_features=1):
        super(KernelRunModel, self).__init__()

        self.ip = torch.nn.Linear(ip_features, num_hidden)
        self.hidden2 = torch.nn.Linear(num_hidden, num_hidden*2)
        self.hidden3 = torch.nn.Linear(num_hidden*2, num_hidden)
        self.hidden4 = torch.nn.Linear(num_hidden, ip_features)
        self.op_run = torch.nn.Linear(ip_features, op_features)
    
    def forward(self, x):
        x = F.relu(self.ip(x))
        x = F.relu(self.hidden2(x))
        x = F.relu(self.hidden3(x))
        x = F.relu(self.hidden4(x))
        x = self.op_run(x)
        return x




dr_columns = ['Kernel'] # using all hardware features except string variables


dataset_root=""
df = pd.read_csv(dataset_root+"new_log_ds.csv")
single = df.drop(columns=dr_columns)


print(list(single.columns))
print(len(single))
print(len(list(single.columns)))
X = single.iloc[:, 0:-1]
y = single.iloc[:, -1]

train_eval_split=0.7
split_seed=42

total_sets = PrepareData(X, y, train=True)
m_scaler = total_sets.return_scaler_obj()
print(len(total_sets))
test_split = 0.2

dataset_size = len(total_sets)
indices = list(range(dataset_size))
random.shuffle(indices)
split = int(np.floor(test_split * dataset_size))
train_indices, test_indices = indices[split:], indices[:split]


#val split
val_split=0.1 # 10% of remaining training data
vsplit = int(np.floor(val_split*(1-test_split)*dataset_size))
val_idx, train_idx = indices[split:split+vsplit], indices[split+vsplit:]

train_sampler = SubsetRandomSampler(train_idx)
val_sampler = SubsetRandomSampler(val_idx)
test_sampler = SubsetRandomSampler(test_indices)
tr_loader = DataLoader(total_sets, batch_size=16, sampler=train_sampler)
val_loader = DataLoader(total_sets, batch_size=1, sampler=val_sampler)
te_loader = DataLoader(total_sets, batch_size=1, sampler=test_sampler)
print('train batches: ', len(tr_loader),' validate samples:', len(val_loader),' test samples:', len(te_loader))




from torch.autograd import Variable

mod = KernelRunModel(33,66).to(device)

def init_weights(m):
    if isinstance(m, nn.Linear):
        torch.nn.init.xavier_normal_(m.weight)
        m.bias.data.fill_(0.00)

mod.apply(init_weights)


criterion = nn.MSELoss(reduction='mean')
criterion2 = nn.L1Loss(reduction='sum')
num_epochs=150

opt = torch.optim.RMSprop(mod.parameters(), lr=1e-2, momentum=0.87, weight_decay=1e-4)
lr_scheduler = torch.optim.lr_scheduler.StepLR(opt, step_size=30, gamma=0.1)

rmse_best=np.inf
mape_best=np.inf
l2l1_best=np.inf
best_model = None
for e in range(num_epochs):
    batch_losses = []
    mod.train()
    for ix, (Xb, yb) in enumerate(tr_loader):
        _X = Variable(Xb).float()
        _y = Variable(yb).float()

        #==========Forward pass===============
        _X = _X.to(device)
        _y = _y.to(device)
        preds = mod(_X)
        loss = criterion(preds, _y)
        loss2 = criterion2(preds, _y)
        total_loss = loss
        #==========backward pass==============

        opt.zero_grad()
        total_loss.backward()
        opt.step()

        batch_losses.append(total_loss.item())
    
    mbl = np.mean(np.sqrt(batch_losses)).round(3)
    
    print("Epoch [{}/{}], Batch loss: {}".format(e, num_epochs, mbl))
    
    mod.eval()
    mape=0.0
    rmse=0.0
    l2l1=0.0
    with torch.no_grad():
        gtr_ = list()
        pred_ = list()
        val_loss = list()
        for index, (xt, yt) in enumerate(val_loader):
            gtr_.append(yt.cpu().data.numpy()[0])

            _xt = Variable(xt).float()
            _yt = Variable(yt).float()

            _xt = _xt.to(device)
            _yt = _yt.to(device)

            predictions = mod(_xt)  ## no need to filter out negative op
            loss1 = criterion(predictions, _yt)
            loss2 = criterion2(predictions, _yt)
            val_loss.append(loss1.item()+loss2.item())

            pred_.append(predictions.cpu().data.numpy()[0])
            #print(predictions, _yt)

        mape = mean_absolute_percentage_error(gtr_, pred_)
        rmse = np.sqrt(mean_squared_error(gtr_, pred_))
        l2l1 = np.mean(np.sqrt(val_loss)).round(3)
        print('Epoch:', e, 'RMSE: ', rmse, ' MAPE:', mape, ' L2+L1 loss:', l2l1)
    
    #if rmse < rmse_best:
    #    rmse_best=rmse
    #    mape_best=mape
    #    l2l1_best=l2l1
    #    best_model=copy.deepcopy(mod)
    
    lr_scheduler.step()

best_model=copy.deepcopy(mod)

print('\n\nEvaluating Model.......')
print('Best Model - RMSE:', rmse_best, ' MAPE:', mape_best, ' L2+L1-', l2l1_best)
#Evaluate model on testing data
best_model.eval()
less_5_gt = list()
less_5_pred = list()

with torch.no_grad():
    total_loss = 0
    gt_ = list()
    preds_ = list()
    
    # custom prediction metric
    less_5_pr = 0
    less_5_gt = 0
    less_100_pr = 0
    less_100_gt = 0
    more_100_pr = 0
    more_100_gt = 0
    print('predicted_runtime, ground_truth')

    for index, (xt, yt) in enumerate(te_loader):
        gt_.append(yt.cpu().data.numpy()[0])
        gr_truth = yt.cpu().data.numpy()[0]


        _xt = Variable(xt).float()
        _yt = Variable(yt).float()
        
        _xt = _xt.to(device)
        _yt = _yt.to(device)
        
        predictions = F.relu(best_model(_xt))
        loss1 = criterion(predictions, _yt)
        preds_.append(predictions.cpu().data.numpy()[0])
        pr_val = predictions.cpu().data.numpy()[0]
        
        if gr_truth <= 5.0:
            less_5_gt += 1
            if abs(gr_truth - pr_val) <= 2.00:
                less_5_pr += 1
        elif gr_truth <= 100.00:
            less_100_gt += 1
            if abs(gr_truth - pr_val) <= 10.00:
                less_100_pr += 1
        else:
            more_100_gt += 1
            if abs(gr_truth-pr_val) <= 0.1*gr_truth:
                more_100_pr +=1

        
        print(predictions.cpu().data.numpy()[0][0],',', _yt.cpu().data.numpy()[0][0])
        total_loss += loss1
    
    mape = mean_absolute_percentage_error(gt_, preds_)
    rmse = np.sqrt(mean_squared_error(gt_, preds_))
    print('RMSE: ', rmse, ' MAPE:', mape)
    print('5: ground truth total- ', less_5_gt, ' predicted total - ', less_5_pr)
    print('100: ground truth total- ', less_100_gt, ' predicted total - ', less_100_pr)
    print(' more 100: ground truth total - ', more_100_gt, ' predicted total - ', more_100_pr)

