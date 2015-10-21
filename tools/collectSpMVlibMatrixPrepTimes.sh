#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

methodName=$1

echo "*" Collecting the filesizes of the spMVlib-generated codes for "$methodName"

fileName="$HOSTNAME".spMVgen.matrixPrepTime."$methodName".csv

rm -f $fileName
while read matrixName
do
    echo -n $matrixName" "
    echo -n $matrixName", " >> $fileName
    cd ..
    rm -f temp.o
    prepTime=`./spMVgen matrices/$matrixName splitAll "$methodName" CSRbyNZ | grep -i "$methodName" | awk '{print $2}'`
    cd - > /dev/null
    echo $prepTime >> $fileName
done < matrixNames.txt

echo " "
echo "* Results written to $fileName"
