#!/bin/bash

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi
   
while read matrixName
do
    cd ..
    echo -n $matrixName" "
    ./build/spMVgen $MATRICES/"$matrixName" PlainCSR -matrix_stats -num_threads 8
    cd - > /dev/null
done < matrixNames.txt
