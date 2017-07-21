#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<thundercat parameters>'" >&2
  exit 1
fi

method1Name=$1
method2Name=$2

echo "*" Comparing spMVlib-generated code for "$method1Name" against spMVlib-generated "$method2Name"

# Run manually-generated code for performance
while read line
do
    IFS=' ' read -r -a info <<< "$line"
    groupName=${info[0]}
    matrixName=${info[1]}

    echo -n "$groupName"/"$matrixName "
    folder1Name=data/"$groupName"/"$matrixName"/"$method1Name"
    folder2Name=data/"$groupName"/"$matrixName"/"$method2Name"
    cd ..
    diff -q "$folder1Name"/output.txt "$folder2Name"/output.txt
    echo ""
    cd tools
done < matrixNames.txt

