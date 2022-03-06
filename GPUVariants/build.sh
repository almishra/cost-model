#!/bin/bash

if [ $# -gt 0 ] && [ "$1" = "clean" ]
then
  echo "Removing and re-creating .build files"
  rm -r .build/
  mkdir .build
  cd .build/
  CC=clang CXX=clang++ cmake -DLLVM_ENABLE_RTTI=true ..
  cd ..
fi

cd .build/
make
ret=$(echo $?)
cd ..

if [ $ret -eq 0 ]
then
  ./test.sh
fi
