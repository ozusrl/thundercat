#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

method1Name=$1
method2Name=$2

echo "*" Comparing spMVlib-generated code for "$method1Name" against spMVlib-generated "$method2Name"

# Run manually-generated code for performance
while read matrixName
do
    echo -n $matrixName" "
    folder1Name=data/"$matrixName"/dynamic_"$method1Name"
    folder2Name=data/"$matrixName"/dynamic_"$method2Name"
    cd ..
    diff -q "$folder1Name"/output.txt "$folder2Name"/output.txt
    cd tools
done < matrixNames.txt

echo " "
