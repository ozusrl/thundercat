#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

methodName=$1
methodParam1=$2
methodParam2=$3

echo "*" Running spMVlib-generated code for "$methodName""$methodParam1""$methodParam2" for CORRECTNESS

# Run manually-generated code for performance
while read matrixName
do
    echo -n $matrixName" "
    if [ "$methodName" == "unrolling" ]; then
	folderName=../../newspecs/"$matrixName"/"$matrixName"_"$methodName"_"$methodParam1"
    elif [ "$methodName" == "OSKI" ]; then
	echo "OSKI not supported by this script yet."
    elif [ "$methodName" == "genOSKI" ]; then
	folderName=../../newspecs/"$matrixName"/"$matrixName"_"$methodName"_"$methodParam1"_"$methodParam2"
    else
	folderName=../../newspecs/"$matrixName"/"$matrixName"_"$methodName"
    fi
    cd ..
    rm -f myoutput2.txt
    ./spMVgen matrices/$matrixName "$methodName" $methodParam1 $methodParam2 -debug > myoutput2.txt
    cd "$folderName"
    ./clang2.out 1 > clangoutput2.txt
    cd - > /dev/null
    diff myoutput2.txt "$folderName"/clangoutput2.txt
    cd tools
done < matrixNames.txt

echo " "
