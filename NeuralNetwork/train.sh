#!/usr/bin/bash

if [ "$#" -eq 0 ]
then
  data_file="cpu_data_log.csv"
else
  data_file=$1
fi

export OMP_NUM_THREADS=20 
python cost.py $data_file
