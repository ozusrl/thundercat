#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi

methodName=$1

echo "*" Collecting the filesizes of the spMVlib-generated codes for "$methodName"

fileName="$HOSTNAME".spMVgen.fileSize."$methodName".csv

rm -f $fileName
while read matrixName
do
    echo -n $matrixName" "
    echo -n $matrixName", " >> $fileName
    cd ..
    rm -f temp.o
    ./build/spMVgen $MATRICES/$matrixName "$methodName" -dump_object > temp.o
    fileSize=`ls -l temp.o | awk '{print $5}'`
    cd - > /dev/null
    echo $fileSize >> $fileName
done < matrixNames.txt

echo " "
echo "* Results written to $fileName"
