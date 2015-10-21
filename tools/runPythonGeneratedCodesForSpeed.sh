#!/bin/bash

methodName=$1
methodParam1=$2
methodParam2=$3

echo "*" Running python-generated code for "$methodName"
while read matrixName
do
    echo -n $matrixName" "
    cd ../../../newspecs/"$matrixName"/"$matrixName"_"$methodName"

    rm -f iccoutput.tx
    ./icc.out >> iccoutput.txt 
    ./icc.out >> iccoutput.txt 
    ./icc.out >> iccoutput.txt 
    cd - > /dev/null
done < matrixNames.txt

echo " "
