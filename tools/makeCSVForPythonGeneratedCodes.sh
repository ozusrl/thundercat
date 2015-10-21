#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <methodName> <methodParams>" >&2
  exit 1
fi

methodName=$1
methodParam1=$2
methodParam2=$3

while read matrixName
do
    echo -n $matrixName", "
    folderName="$matrixName"/"$matrixName"_"$methodName"
    
    values=`cat ../../../newspecs/$folderName/iccoutput.txt`
    #echo $values
    ./findMinTiming.py $values
    echo ""
done < matrixNames.txt

