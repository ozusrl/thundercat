#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 '<spMVlib params>'" >&2
  exit 1
fi

methodName=$1
methodParams=$2

while read matrixName
do
    echo -n $matrixName", "
    cd ../../../newspecs/"$matrixName"/spMVgen_"$matrixName"_"$methodName""$methodParams"
    values=`cat runtime.txt`
    cd - > /dev/null
    ./findMinTiming.py $values
done < matrixNames.txt

