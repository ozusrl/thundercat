#!/bin/bash

# prints matrixName, N, NZ, singleRowStencils,
#  multiRowStencils, sumOfStencilLengths, NZGroups,
#  sumOfNZLengths, numDistinctVals, avgNZperRow

while read matrixName
do
    cd ..
    echo -n $matrixName" "
    ./spMVgen matrices/"$matrixName" PlainCSR -matrix_stats -num_threads 8
    cd - > /dev/null
done < matrixNames.txt