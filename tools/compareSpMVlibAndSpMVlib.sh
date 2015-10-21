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
    folder1Name=../../newspecs/spMVgen/"$matrixName"/"$method1Name"
    folder2Name=../../newspecs/spMVgen/"$matrixName"/"$method2Name"
    cd ..
    ndiff --relative-error 1.0e-05 "$folder1Name"/output.txt "$folder2Name"/output.txt 
    cd tools
done < matrixNames.txt

echo " "
