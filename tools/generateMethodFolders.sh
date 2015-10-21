#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<method name>'" >&2
  exit 1
fi

methodName=$1
methodParam1=$2
methodParam2=$3

echo "*" Generating folders for "$methodName""$methodParam1""$methodParam2"
while read matrixName
do
    echo -n $matrixName" "
    cd ../../../newspecs
    if [ "$methodName" == "unrolling" ]; then
	rm -rf "$matrixName"/"$matrixName"_"$methodName"_"$methodParam1"
	python specialize.py "$matrixName" "[unrolling, $methodParam1]"
    elif [ "$methodName" == "OSKI" ]; then
	echo "OSKI not supported by this script yet."
    elif [ "$methodName" == "genOSKI" ]; then
	rm -rf "$matrixName"/"$matrixName"_"$methodName"_"$methodParam1"_"$methodParam2"
	python specialize.py "$matrixName" "[genOSKI, $methodParam1, $methodParam2]"
    else
	rm -rf "$matrixName"/"$matrixName"_"$methodName"
	python specialize.py "$matrixName" $methodName
    fi
    cd - > /dev/null
done < matrixNames.txt

echo " "
