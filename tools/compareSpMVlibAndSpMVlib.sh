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
    folder1Name=data/"$matrixName"/"$method1Name"
    folder2Name=data/"$matrixName"/"$method2Name"
    cd ..
    numdiff -q "$folder1Name"/output.txt "$folder2Name"/output.txt numdiff.cfg
    cd tools
done < matrixNames.txt

echo " "
