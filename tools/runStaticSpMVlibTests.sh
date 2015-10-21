#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

methodName=$1
methodParam1=$2
methodParam2=$3

echo "Running spMVlib test " $methodName $methodParam1 $methodParam2

while read matrixName
do
    echo -n $matrixName" "
    folderName=../../../newspecs/spMVgen/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    mkdir -p "$folderName"
    rm -f "$folderName"/output.txt
    rm -f "$folderName"/genCost.txt
    rm -f "$folderName"/runtime.txt
    rm -f "$folderName"/matrixPrepTime.txt

    cd ..
    ./spMVgen matrices/$matrixName $methodName $methodParam1 $methodParam2 -dump_matrix > tools/"$folderName"/decls.c
    ./spMVgen matrices/$matrixName $methodName $methodParam1 $methodParam2 -dump_object > tools/"$folderName"/multByMs.o
    cp tools/driver"$methodParam2".c tools/"$folderName"/
    cp tools/main.c tools/"$folderName"/
    cp tools/main-debug.c tools/"$folderName"/
    cd tools

    cd "$folderName"
    icc -c -O3 -fopenmp driver"$methodParam2".c
    clang -c -O0 decls.c
    icc -c -O3 main.c
    icc -O3 -fopenmp *.o -o parallel
    icc -c -O3 main-debug.c -o main.o
    icc -O3 -fopenmp *.o -o parallel-debug

    ./parallel >> runtime.txt
    ./parallel >> runtime.txt
    ./parallel >> runtime.txt

    ./parallel-debug >> output.txt

    rm *.o *.c parallel parallel-debug

    cd - > /dev/null

done < matrixNames.txt

echo " "

findMins() {
    local fileName=$HOSTNAME.spMVgen."$methodName""$methodParam1""$methodParam2".csv 
    rm -f $fileName
    while read matrixName
    do
	cd ../../../newspecs/spMVgen/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
	runTimes=`cat runtime.txt`
	cd - > /dev/null
	echo -n $matrixName" "  >> $fileName
	./findMinTiming.py $runTimes >> $fileName
	echo     ""  >> $fileName
    done < matrixNames.txt
}

findMins
