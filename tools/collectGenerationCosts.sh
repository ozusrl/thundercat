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

methodName=$1
methodParam1=$2
methodParam2=$3

echo "Running spMVlib test " $methodName $methodParam1 $methodParam2

while read matrixName
do
    echo -n $matrixName" "

    cd ..
    folderName=data/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    mkdir -p "$folderName"
    rm -f "$folderName"/stats*.txt

    ./build/thundercat $MATRICES/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 -iters 1 > "$folderName"/stats1.txt
    ./build/thundercat $MATRICES/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 -iters 1 > "$folderName"/stats2.txt
    ./build/thundercat $MATRICES/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 -iters 1 > "$folderName"/stats3.txt
    cd tools

done < matrixNames.txt

echo " "
    
findMins() {
    currentTime=`date +%Y.%m.%d`
    local fileName=../data/$HOSTNAME.gencost.$currentTime."$methodName""$methodParam1""$methodParam2".csv 
    rm -f $fileName
    while read matrixName
    do
        toolsFolder=`pwd`
        cd ../data/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
        codeGen1=`grep generateFunctions stats1.txt | awk '{print $2}'`
        codeGen2=`grep generateFunctions stats2.txt | awk '{print $2}'`
        codeGen3=`grep generateFunctions stats3.txt | awk '{print $2}'`
        indexOfMin=`$toolsFolder/findIndexOfMin.py $codeGen1 $codeGen2 $codeGen3`
        statsFile=stats"$indexOfMin".txt
        partitionTime=`grep getStripeInfos $statsFile | awk '{print $2}'`
        analysisTime=`grep analyzeMatrix $statsFile | awk '{print $2}'`
        conversionTime=`grep convertMatrix $statsFile | awk '{print $2}'`
        setMultByMFunctionsTime=`grep setMultByMFunctions $statsFile | awk '{print $2}'`
        if [ "$setMultByMFunctionsTime" = "" ]; then
            setMultByMFunctionsTime=0
        fi
        emissionTime=`grep emitCode $statsFile | awk '{print $2}'`
        codeGenTime=`grep generateFunctions $statsFile | awk '{print $2}'`
        cd - > /dev/null
        echo $matrixName $partitionTime $analysisTime $conversionTime $setMultByMFunctionsTime $emissionTime $codeGenTime  >> $fileName
    done < matrixNames.txt
}

findMins

echo "$0 $1 $2 $3 on $HOSTNAME has finished." | mail -s "$0 $1 $2 $3 on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
