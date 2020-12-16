#!/bin/bash
export OMP_NUM_THREADS=4
for i in `ls | grep -v "\."`
do
  if [[ $i != "Makefile" ]]
  then 
    echo -ne "$i,"
    ./$i
  fi
done
