#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<numThreads>'" >&2
  exit 1
fi

numThreads=$1


echo "Running static MKL test " $numThreads

while read matrixName
do
    echo -n $matrixName" "
    folderName=../../../newspecs/spMVgen/"$matrixName"/MKL"$numThreads"
    mkdir -p "$folderName"
    rm -f "$folderName"/output.txt
    rm -f "$folderName"/genCost.txt
    rm -f "$folderName"/runtime.txt
    rm -f "$folderName"/matrixPrepTime.txt

    cd ..
    ./spMVgen matrices/$matrixName MKL -num_threads $numThreads -dump_matrix > tools/"$folderName"/decls.c
    cp tools/mklDriver.c tools/"$folderName"/
    cp tools/main.c tools/"$folderName"/
    cp tools/main-debug.c tools/"$folderName"/
    cd tools

    cd "$folderName"
    sed -i.bak 's/int numMatrixRows/int n/g' decls.c
    sed -i.bak 's/int numMatrixValues/unsigned long nz/g' decls.c
    sed -i.bak "s/NUMTHREADS/$numThreads/g" mklDriver.c
    rm *.bak
    icc -c -O3 -mkl mklDriver.c
    clang -c -O0 decls.c
    icc -c -O3 main.c
    icc -O3 -mkl *.o -o parallel
    icc -c -O3 main-debug.c -o main.o
    icc -O3 -mkl *.o -o parallel-debug

    ./parallel >> runtime.txt
    ./parallel >> runtime.txt
    ./parallel >> runtime.txt

    ./parallel-debug >> output.txt

    rm *.o *.c parallel parallel-debug

    cd - > /dev/null

done < matrixNames.txt

echo " "

findMins() {
    local fileName=$HOSTNAME.spMVgen.MKL"$numThreads".csv 
    rm -f $fileName
    while read matrixName
    do
	cd ../../../newspecs/spMVgen/"$matrixName"/MKL"$numThreads"
	runTimes=`cat runtime.txt`
	cd - > /dev/null
	echo -n $matrixName" "  >> $fileName
	./findMinTiming.py $runTimes >> $fileName
	echo     ""  >> $fileName
    done < matrixNames.txt
}

findMins

echo "Static MKL $numThreads test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr  