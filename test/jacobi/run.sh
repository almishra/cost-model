#!/bin/bash
export OMP_NUM_THREADS=28
for i in `ls | grep -v "\."`
do
  if [[ $i != "Makefile" ]]
  then 
    echo -ne "$i,"
    ./$i
  fi
done
