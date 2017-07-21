#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<thundercat parameters>'" >&2
  exit 1
fi

if [ -z ${MATRICES+x} ]; then
  echo "Set MATRICES variable to the matrices folder."
  exit 1
fi

source /opt/intel/bin/compilervars.sh intel64

mkdir -p ../data

methodName=$1
methodParam1=$2
methodParam2=$3

test() {
    local groupName=$1
    local matrixName=$2
    local methodName=$3
    local methodParam1=$4
    local methodParam2=$5

    folderName=data/"$groupName"/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    mkdir -p "$folderName"
    rm -f "$folderName"/output.txt

    ./build/thundercat $MATRICES/$groupName/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 -debug > "$folderName"/output.txt
}

echo "*" Running spMVlib for "$methodName""$methodParam1""$methodParam2"

while read line
do
    IFS=' ' read -r -a info <<< "$line"
    groupName=${info[0]}
    matrixName=${info[1]}

    echo -n "$groupName"/"$matrixName "
    cd ..
    test $groupName $matrixName "$methodName" "$methodParam1" "$methodParam2"
    cd - > /dev/null
done < matrixNames.txt

echo " "
