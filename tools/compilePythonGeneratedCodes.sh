#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<method name>'" >&2
  exit 1
fi

methodName=$1

echo "*" Compiling python-generated code for "$methodName"
while read matrixName
do
    echo -n $matrixName" "
    cd ../../../newspecs/"$matrixName"/"$matrixName"_"$methodName"

    cp ../../makefile .
    cp ../../main.c ./main.c
    rm -f a.out
    make -j6
    cp a.out icc.out
    #rm *.o
    #cp ../../main2.c ./main.c
    #make >& /dev/null
    #mv a.out clang2.out
    cd - > /dev/null
done < matrixNames.txt

echo " "
