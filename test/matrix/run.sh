#!/bin/bash
for i in `ls | grep -v "\."`
do
  if [[ $i != "Makefile" ]]
  then 
    echo -ne "$i,"
    ./$i
  fi
done
