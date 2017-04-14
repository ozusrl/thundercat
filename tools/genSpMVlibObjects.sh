#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<thundercat parameters>'" >&2
  exit 1
fi

if [ -z ${MATRICES+x} ]; then
  echo "Set MATRICES variable to the matrices folder."
  exit 1
fi

mkdir -p ../data

methodName=$1
methodParam1=$2
methodParam2=$3

test() {
    local matrixName=$1
    local methodName=$2
    local methodParam1=$3
    local methodParam2=$4

    folderName=data/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    mkdir -p "$folderName"
    rm -f "$folderName"/generated_*
    rm -f ./build/generated_* ./generated_*

    ./build/thundercat $MATRICES/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 -dump_object > /dev/null
    mv ./generated_* "$folderName"/
}

echo "*" Running spMVlib for "$methodName""$methodParam1""$methodParam2"

while read matrixName
do
    echo -n $matrixName" "

    cd ..
    test $matrixName "$methodName" "$methodParam1" "$methodParam2"
    cd - > /dev/null
done < matrixNames.txt

echo " "
