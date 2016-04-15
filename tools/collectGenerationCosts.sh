#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi

methodName=$1
methodParam1=$2
methodParam2=$3

runTests() {
    echo "Running spMVlib test " $methodName $methodParam1 $methodParam2

    while read matrixName
    do
        echo -n $matrixName" "
        folderName=../data/"$matrixName"/dynamic_"$methodName""$methodParam1""$methodParam2"
        mkdir -p "$folderName"
        
        cd ..
        ./build/spMVgen $MATRICES/$matrixName $methodName $methodParam1 $methodParam2 > tools/"$folderName"/stats1.txt
        ./build/spMVgen $MATRICES/$matrixName $methodName $methodParam1 $methodParam2 > tools/"$folderName"/stats2.txt
        ./build/spMVgen $MATRICES/$matrixName $methodName $methodParam1 $methodParam2 > tools/"$folderName"/stats3.txt
        cd tools
        
    done < matrixNames.txt
    
    echo " "
}
    
findMins() {
    local fileName=$HOSTNAME.spMVgen.gencost."$methodName""$methodParam1""$methodParam2".csv 
    rm -f $fileName
    while read matrixName
    do
        toolsFolder=`pwd`
        cd ../data/"$matrixName"/dynamic_"$methodName""$methodParam1""$methodParam2"
        codeGen1=`grep codeGeneration stats1.txt | awk '{print $2}'`
        codeGen2=`grep codeGeneration stats2.txt | awk '{print $2}'`
        codeGen3=`grep codeGeneration stats3.txt | awk '{print $2}'`
        indexOfMin=`$toolsFolder/findIndexOfMin.py $codeGen1 $codeGen2 $codeGen3`
        statsFile=stats"$indexOfMin".txt
        infoTime=`grep Info $statsFile | awk '{print $2}'`
        conversionTime=`grep Conversion $statsFile | awk '{print $2}'`
        getMatrixTime=`grep getMatrix $statsFile | awk '{print $2}'`
        emitCodeTime=`grep emitCode $statsFile | awk '{print $2}'`
        codeGenTime=`grep codeGeneration $statsFile | awk '{print $2}'`
        cd - > /dev/null
        echo $matrixName $infoTime $conversionTime $getMatrixTime $emitCodeTime $codeGenTime  >> $fileName
    done < matrixNames.txt
}

runTests
findMins
